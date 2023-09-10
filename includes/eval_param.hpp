# ifdef TUNE_EVAL
# include "register_params.hpp"
# define EVAL_PARAM REGISTER_TUNABLE_PARAM
# define EVAL_PARAM_ARRAY REGISTER_TUNABLE_PARAM_ARRAY
# else
# define EVAL_PARAM(name, value) const int name = value;
# define EVAL_PARAM_ARRAY(size, name, ...) const std::array<int, size> name = { __VA_ARGS__ };
# endif
