#ifndef NEXUSUTILS_H
#define NEXUSUTILS_H
# include <cstdlib>
# include <string>
namespace nexusutils
{
const int FILETYPE_NEXUS=100;
const int FILETYPE_NEWICK=200;
const int INVALID_NEWICK_STRING=300;
const int VALID_NEWICK_STRING=400;

std::string get_newickstring(std::string str);
}
#endif // NEXUSUTILS_H
