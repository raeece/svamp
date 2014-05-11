#include "nexusutils.h"
#include <ncl/ncl.h>

namespace nexusutils
{

std::string get_newickstring(std::string fileStr){

       MultiFormatReader nexusReader(-1, NxsReader::IGNORE_WARNINGS);

       nexusReader.SetWarningOutputLevel(8);
       nexusReader.SetAlwaysReportStatusMessages(false);
       nexusReader.ReadStringAsNexusContent(fileStr);
       NxsTaxaBlock * taxaBlock = nexusReader.GetTaxaBlock(0);
       unsigned nTreeBlocks = nexusReader.GetNumTreesBlocks(taxaBlock);
       NxsTreesBlock * treeBlock = nexusReader.GetTreesBlock(taxaBlock,0);
       NxsString treeStr = treeBlock->GetTranslatedTreeDescription(0);
       return (std::string)treeStr;
}

}
