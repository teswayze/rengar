# include <string>
# include <array>
# include <tuple>
# include <queue>

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
	size_t param_id;
	size_t index;
	int proposed_mod;
};

std::string tweak_to_string(const ProposedTweak tweak);
void apply_tweak(const ProposedTweak tweak);
void unapply_tweak(const ProposedTweak tweak);

struct TweakWithPriority{
	int priority;
	ProposedTweak tweak;
};

constexpr bool operator<(const TweakWithPriority x, const TweakWithPriority y){
	return x.priority < y.priority;
}

using TweakQueue = std::priority_queue<TweakWithPriority>;
TweakQueue initialize_queue(const int starting_mod);
