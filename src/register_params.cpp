# include "register_params.hpp"
# include <vector>

struct ParamNameAndReference{
	const std::string name;
	int* const reference;
};

static std::vector<ParamNameAndReference>& mutable_params(){
	static std::vector<ParamNameAndReference> v;
	return v;
}

void register_param(std::string param_name, int *param_ref){
	mutable_params().push_back(ParamNameAndReference{param_name, param_ref});
}


# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

REGISTER_MUTABLE_PARAM(test_param, 3);

TEST_CASE("Mutable parameter"){
	CHECK(test_param == 3);
	CHECK(mutable_params().size() == 1);
	CHECK(mutable_params()[0].name == "test_param");
	
	*mutable_params()[0].reference += 1;
	CHECK(test_param == 4);
	*mutable_params()[0].reference -= 1;
	CHECK(test_param == 3);
}

# endif
