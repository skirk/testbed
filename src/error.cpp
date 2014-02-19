#include "error.hpp"

void interrupt(std::string _errormessage, int _errorcode) {
	std::cout<<_errormessage<<" "<<getCLErrorString(_errorcode)<<'\n';
	CL_Resources &res=  CL_Resources::getInstance();
	res.release();
	exit(1);
}
