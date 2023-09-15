# pragma once

# include <string>
# include <array>
# include <tuple>
# include <queue>
# include <optional>

void register_param(std::string param_name, int *param_ref, size_t length);

# define REGISTER_TUNABLE_PARAM(name, value)						\
    int name = value;                                               \
    struct _temp_class_##name                                       \
    {                                                               \
    	_temp_class_##name() { register_param(#name, &name, 1); }   \
    } _temp_instance_##name;

# define REGISTER_TUNABLE_PARAM_ARRAY(size, name, ...)    		    	   \
	std::array <int, size> name = { __VA_ARGS__ };						   \
	struct _temp_class_##name                                       	   \
    {                                                              		   \
    	_temp_class_##name() { register_param(#name, name.data(), size); } \
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
	uint64_t priority;
	ProposedTweak tweak;
};

constexpr bool operator<(const TweakWithPriority x, const TweakWithPriority y){
	return x.priority < y.priority;
}

using TweakQueue = std::priority_queue<TweakWithPriority>;
TweakQueue initialize_queue(const int starting_mod);

void show_current_param_values(std::string param_kind);
std::optional<size_t> get_param_id_by_name(const std::string param_name);
