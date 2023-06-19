# include <string>
# include <array>

void register_param(std::string param_name, int *param_ref, size_t length);

# define REGISTER_TUNABLE_PARAM(name, value)						\
    int name = value;                                               \
    struct _temp_class_##name                                       \
    {                                                               \
    	_temp_class_##name() { register_param(#name, &name, 1); }   \
    } _temp_instance_##name;

# define REGISTER_TUNABLE_PARAM_ARRAY(size, name, ...)    		    \
	int name[] = { __VA_ARGS__ };									\
	struct _temp_class_##name                                       \
    {                                                               \
    	_temp_class_##name() { register_param(#name, name, size); } \
    } _temp_instance_##name;

struct ProposedTweak{
	const size_t param_id;
	const size_t index;
	const int proposed_mod;
};

std::string tweak_to_string(const ProposedTweak tweak);
void apply_tweak(const ProposedTweak tweak);
void unapply_tweak(const ProposedTweak tweak);
