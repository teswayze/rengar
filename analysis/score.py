import torch


def loss_function(net_output: torch.FloatTensor, game_result: torch.FloatTensor) -> torch.FloatTensor:
    """
        For white win: loss = sqrt(eval^2 + 1) - eval
        For black win: loss = sqrt(eval^2 + 1) + eval
        For draw: loss = sqrt(eval^2 + 1) - 1

        Properties:
        - Loss for decisive game is monotonic, tending to zero for large evaluations with the correct sign
        - Loss for draw has local minimum at (0, 0)
        - Loss function is convex --> gradient is monotonic
        - Loss gradient is bounded between +/- 2
        - A win + loss has same effect as a 2 draws on training
        
        Implied expected game score for white = 0.5 + 0.5 * eval / sqrt(eval^2 + 1)
        Using 256cp = 1:
        - eval = 52cp --> white scores 60%
        - eval = 112cp --> white scores 70%
        - eval = 192cp --> white scores 80%
        - eval = 341cp --> white scores 90%
        - eval = 529cp --> white scores 95%

    """
    loss_factor = (1 + net_output ** 2) ** 0.5
    return torch.mean(loss_factor - game_result * net_output - game_result == 0)
