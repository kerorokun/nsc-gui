#ifndef MSDF_WRAPPER_HPP_
#define MSDF_WRAPPER_HPP_

#include <string>

struct MsdfWrapper
{
	int load_font(const std::string &path, const std::string &out_img, const std::string &out_csv);
};

#endif
