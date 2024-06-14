#ifndef M3U8_PARSER_H
#define M3U8_PARSER_H
#include <vector>
#include <string>
#include <sstream>

#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
using namespace std;
void split(const string& src, const string& separator, vector<string>& dest)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;

    do
    {
        index = str.find_first_of(separator,start);
        if (index != string::npos)
        {
            substring = str.substr(start,index-start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator,index);
            if (start == string::npos) return;
        }
    }while(index != string::npos);

    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
}

using namespace std;
class M3u8Parser
{

public:
    struct M3u8Item
    {
        string title;
        uint64_t dur;
        string tsLoc;
    };
    struct M3u8
    {
        // m3u8  header
        uint64_t dur;
        // m3u8 items
        vector<M3u8Item> itsms;
    };
    M3u8Parser(){}
    ~M3u8Parser(){}
    M3u8 parse(const string& m3u8)
    {
        M3u8 m3;
        vector<string> tok;
        split(m3u8, "\n", tok);
        for(vector<string>::iterator itr = tok.begin(); itr != tok.end(); ++itr)
        {
            string s = *itr;
            string::size_type loc;
            if (string::npos !=  s.find("#EXTINF:"))
            {
                loc = s.find(":");
                M3u8Item itm;
                s = s.substr(loc+1);
                string::size_type commaLoc = s.find(',');
                // extra dur
                string dur = s.substr(0, commaLoc);
                istringstream istr(dur);
                istr >> itm.dur;
                // extra title(url)
                itm.title = s.substr(commaLoc+1);
                itm.tsLoc = *(++itr);
                m3.itsms.push_back(itm);

            }
        }
        return m3;

    }

};
#endif


