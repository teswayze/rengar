# include "doctest.h"
# include "../tune/register_params.hpp"

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
	auto app_queue = initialize_queue(8);
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
