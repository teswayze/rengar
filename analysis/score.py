import torch


class LossFunction(torch.nn.Module):
    """
        For white win: loss = softplus(-sign(x) * |x|^p) ^ q
        For black win: loss = softplus( sign(x) * |x|^p) ^ q
        For draw, take the mean

        Properties:
        - Loss for decisive game is monotonic, tending to zero for large evaluations with the correct sign
        - Loss for draw has global minimum at 0
        - Decisive loss only convex for p = 1 and q >= 1
        - Draw loss convex for q >= 1
        - For q=1, the implied game score is 0.5 + 0.5 * tanh(sign(x) * |x|^p)
        - q > 1 upweights games with large eval; q < 1 upweights games with eval close to zero
        - Loss gradient is bounded by O(|x|^(p + q - 2))
        - A win + loss has same effect as a 2 draws on training
    """
    def __init__(self, p: float, q: float):
        assert 1 <= p
        assert 0 < q
        super().__init__()
        self.p = p
        self.q = q

    def softplus(self, input: torch.FloatTensor) -> torch.FloatTensor:
        return torch.where(input >= -100, torch.nn.Softplus()(input), 0.0)
    
    def forward(self, input: torch.FloatTensor, target: torch.FloatTensor) -> torch.FloatTensor:
        input_p = torch.sign(input) * torch.abs(input) ** self.p
        ww_loss = self.softplus(-input_p) ** self.q
        bw_loss = self.softplus(input_p) ** self.q
        draw_loss = (ww_loss + bw_loss) / 2
        loss_diff = (ww_loss - bw_loss) / 2
        return torch.mean(draw_loss + loss_diff * target)
        

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
