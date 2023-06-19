# include "register_params.hpp"
# include <vector>
# include <optional>

struct ParameterDetails{
	const std::string name;
	int* const reference;
	const size_t length;
};

std::vector<ParameterDetails>& mutable_params(){
	static std::vector<ParameterDetails> v;
	return v;
}

void register_param(const std::string param_name, int *param_ref, const size_t length){
	mutable_params().push_back(ParameterDetails{param_name, param_ref, length});
}

std::optional<size_t> get_param_id_by_name(const std::string param_name){
	auto v = mutable_params();
	for (size_t i = 0; i < v.size(); i++){
		if (v[i].name == param_name) return i;
	}
	return std::nullopt;
}

std::string tweak_to_string(const ProposedTweak tweak){
	return mutable_params()[tweak.param_id].name + "[" + std::to_string(tweak.index) + "] += " + std::to_string(tweak.proposed_mod);
}

void apply_tweak(const ProposedTweak tweak){
	mutable_params()[tweak.param_id].reference[tweak.index] += tweak.proposed_mod;
}

void unapply_tweak(const ProposedTweak tweak){
	mutable_params()[tweak.param_id].reference[tweak.index] -= tweak.proposed_mod;
}


TweakQueue initialize_queue(const int starting_mod){
	TweakQueue queue;
	auto v = mutable_params();
	for (size_t i = 0; i < v.size(); i++){
		auto details = v[i];
		for (size_t j = 0; j < details.length; j++){
			queue.push(TweakWithPriority{INT_MAX, ProposedTweak{i, j, starting_mod}});
		}
	}
	return queue;
}

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

REGISTER_TUNABLE_PARAM(test_param, 3)
REGISTER_TUNABLE_PARAM_ARRAY(3, test_param_arr,
		6, 24, 23,
)

TEST_CASE("Tunable parameter"){
	CHECK(test_param == 3);

	auto param_id = get_param_id_by_name("test_param");
	CHECK(param_id.has_value());

	auto tweak = ProposedTweak{param_id.value(), 0, 1};
	apply_tweak(tweak);
	CHECK(test_param == 4);
	unapply_tweak(tweak);
	CHECK(test_param == 3);
}

TEST_CASE("Tunable math"){
	CHECK(test_param_arr[0] == 6);
	CHECK(test_param_arr[1] == 24);
	CHECK(test_param_arr[2] == 23);

	auto param_id = get_param_id_by_name("test_param_arr");
	CHECK(param_id.has_value());

	auto tweak = ProposedTweak{param_id.value(), 1, -1};
	apply_tweak(tweak);
	CHECK(test_param_arr[1] == 23);
	unapply_tweak(tweak);
	CHECK(test_param_arr[1] == 24);
}

TEST_CASE("Initializing queue"){
	auto app_queue = initialize_queue(-8);
	TweakQueue unapp_queue;

	int i = 0;
	while (!app_queue.empty()){
		apply_tweak(app_queue.top().tweak);
		unapp_queue.push(app_queue.top());
		app_queue.pop();
		i++;
	}
	CHECK(i == 4);
	CHECK(test_param == -5);
	CHECK(test_param_arr[0] == -2);
	CHECK(test_param_arr[1] == 16);
	CHECK(test_param_arr[2] == 15);

	while (!unapp_queue.empty()){
		unapply_tweak(unapp_queue.top().tweak);
		unapp_queue.pop();
	}
	CHECK(test_param == 3);
	CHECK(test_param_arr[0] == 6);
	CHECK(test_param_arr[1] == 24);
	CHECK(test_param_arr[2] == 23);
}

# endif
