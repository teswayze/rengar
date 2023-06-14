# include <string>

void register_param(std::string param_name, int *param_ref);

# define REGISTER_MUTABLE_PARAM(name, value)             \
    int name = value;                                    \
    namespace                                            \
    {                                                    \
    struct _temp_class_##name                            \
    {                                                    \
    	_temp_class_##name() { register_param(#name, &name); }    \
    } _temp_instance_##name;                                                 \
    }
