# include <algorithm>
# include <exception>

template <char result>
inline int loss_gradient(const int eval){
    const int clamped_eval = std::clamp(eval, -512, 512);
    int loss = -clamped_eval;
    const int concave_part = ((clamped_eval * clamped_eval) / 1024 + 256);
    if (result == 'W') loss += concave_part;
    if (result == 'B') loss -= concave_part;
    return loss;
}
