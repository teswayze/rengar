import torch


class LossFunction(torch.nn.Module):
    """
        For white win: loss = (|x|^p + a^p)^(q/p) - |x|^q * sign(x)
        For black win: loss = (|x|^p + a^p)^(q/p) - |x|^q * sign(x)
        For draw: loss = (|x|^p + a^p)^(q/p) - a^q
        Expected game score: 0.5 + 0.5 * |x|^(p-q) * sign(x) / (|x|^p + a^p)^(1 - q/p)

        Properties:
        - Loss for decisive game is monotonic, tending to zero for large evaluations with the correct sign
        - Loss for draw has global minimum at (0, 0)
        - Loss function is convex --> gradient is monotonic
        - Loss gradient is bounded by O(|x|^(q-1))
        - A win + loss has same effect as a 2 draws on training
        - a is a scaling parameter - you should get similar weights except for the last layer if you change it
    """
    def __init__(self, a: float, p: float, q: float):
        assert 0 < a
        assert 0 < q
        assert 1 < p
        assert q < p

        super().__init__()
        self.a = a
        self.p = p
        self.q = q
    
    def forward(self, input: torch.FloatTensor, target: torch.FloatTensor) -> torch.FloatTensor:
        main_term = (input.abs() ** self.p + self.a ** self.p) ** self.q / self.p
        decisive_term = target * input.sign() * input.abs() ** self.q
        draw_term = (target == 0) * self.a ** self.q
        return torch.mean(main_term - decisive_term - draw_term)
    
    def expected_target(self, input: torch.FloatTensor) -> torch.FloatTensor:
        return (
            input.sign() * input.abs() ** (self.p - self.q) *
            (input.abs() ** self.p + self.a ** self.p) ** (self.q / self.p - 1)
        )


def sorted_eval_score(input: torch.FloatTensor, target: torch.FloatTensor) -> torch.FloatTensor:
    # This is what we REALLY care about: winning eval > draw eval > loss eval
    # We can't actually use this as a loss function, but we can validate against it and tune to it
    n, = input.size()
    random_noise_score = (n-1) / 2 * torch.mean(target).item()
    # Double argsort -> rank
    # https://stackoverflow.com/questions/5284646/rank-items-in-an-array-using-python-numpy-without-sorting-array-twice
    perfect_score = torch.mean(torch.argsort(torch.argsort(target)) * target).item()
    eval_score = torch.mean(torch.argsort(torch.argsort(input, stable=True)) * target).item()
    return (eval_score - random_noise_score) / (perfect_score - random_noise_score)
