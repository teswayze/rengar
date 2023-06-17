# include "register_params.hpp"
# include <vector>
# include <optional>

struct ParameterDetails{
	const std::string name;
	int* const reference;
	size_t length;
};

static std::vector<ParameterDetails>& mutable_params(){
	static std::vector<ParameterDetails> v;
	return v;
}

void register_param(const std::string param_name, int *param_ref, const size_t length){
	mutable_params().push_back(ParameterDetails{param_name, param_ref, length});
}

std::optional<ParameterDetails> get_param_ref_by_name(const std::string param_name){
	for (auto details : mutable_params()){
		if (details.name == param_name) return details;
	}
	return std::nullopt;
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

REGISTER_TUNABLE_PARAM(test_param, 3);
REGISTER_TUNABLE_PARAM_ARRAY(3, test_param_arr, 6, 24, 23);

TEST_CASE("Tunable parameter"){
	CHECK(test_param == 3);

	auto result = get_param_ref_by_name("test_param");
	CHECK(result.has_value());
	auto details = result.value();

	CHECK(details.name == "test_param");
	CHECK(details.reference == &test_param);
	CHECK(details.length == 1);

	*details.reference += 1;
	CHECK(test_param == 4);
	*details.reference -= 1;
	CHECK(test_param == 3);
}

TEST_CASE("Tunable math"){
	CHECK(test_param_arr[0] == 6);
	CHECK(test_param_arr[1] == 24);
	CHECK(test_param_arr[2] == 23);

	auto result = get_param_ref_by_name("test_param_arr");
	CHECK(result.has_value());
	auto details = result.value();

	CHECK(details.name == "test_param_arr");
	CHECK(details.reference == test_param_arr);
	CHECK(details.length == 3);

	*details.reference += 1;
	CHECK(test_param_arr[0] == 7);
	*details.reference -= 1;
	CHECK(test_param_arr[0] == 6);
}

# endif
