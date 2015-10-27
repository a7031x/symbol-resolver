

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <future>
#include "string_algo.hpp"

using namespace string_algo;
using namespace std;

vector<char> read_file(const string& path)
{
    ifstream file(path, ios::in | ios::binary | ios::ate);
    if(false == file.is_open())
        return vector<char>();
    auto size = file.tellg();
    vector<char> buf(size);
    file.seekg(0, ios::beg);
    file.read(buf.data(), size);
    file.close();

    return buf;
}

void write_file(const string& path, const vector<char>& buf)
{
    ofstream of(path, ofstream::trunc | ofstream::binary);
    if(of.is_open())
    {
        of.write(buf.data(), buf.size());
        of.close();
    }
}

string generate_new_symbol(const string& symbol)
{
    string nsymbol;
    for(auto c : symbol)
    {
        if('_' == c)
            nsymbol += c;
        else if('A' <= c && 'Z' >= c)
            nsymbol += 'A' + (rand() % 26);
        else if('a' <= c && 'z' >= c)
            nsymbol += 'a' + (rand() % 26);
    }
    return nsymbol;
}

bool change_symbol(vector<char>& buf, const string& symbol)
{
    auto pos = buf.begin();
    bool change_flag = false;
    while(true)
    {
        pos = search(pos, buf.end(), symbol.begin(), symbol.end());
        if(buf.end() != pos)
        {
            auto nsymbol = generate_new_symbol(symbol);
            copy(nsymbol.begin(), nsymbol.end(), pos);
            change_flag = true;
            pos += symbol.size();
        //    cout << "rename " << symbol << " to " << nsymbol << " at offset " << pos - buf.begin() << endl;
        }
        else
            break;
    }
    return change_flag;
}

int main(int argc, const char * argv[]) {

    //just copy the error message from xcode to error.txt
    //the following code will parse the message and deside which sdk need to rename its reference.
    ifstream script("error.txt");
    //the output after parsing will generate of symbol map, the key is the sdk file path,
    //the value is the conflicting symbols.
    map<string, vector<string>> symbol_files;
    
    while(false == script.eof())
    {
        char line[10240];
        script.getline(line, sizeof(line) - 1);
        string dup_sym = "duplicate symbol ";
        if(false == istartsWith(line, dup_sym))
            continue;
        auto line_end = RangeIter::end(line);
        auto symbol_start = ifind_after(RangeIter::begin(line), line_end, dup_sym);
        string in = " in:";
        auto symbol_end = search(symbol_start, line_end, in.begin(), in.end());
        if(line_end == symbol_start || line_end == symbol_end)
            continue;
        string symbol(symbol_start, symbol_end);
        script.getline(line, sizeof(line));
        script.getline(line, sizeof(line));
        const char* path = line;
        while(' ' == *path) ++path;
        symbol_files[path].push_back(symbol);
    }
    script.close();
    
    //rename the symboles and the references.
    for(auto kv : symbol_files)
    {
        auto buf = read_file(kv.first); //kv.first is the path of the sdk.
        size_t index = 0;
        if(0 == buf.size())
            continue;
        bool flag = false;
        vector<future<bool>> futs;
        
        //kv.second is the conflicting symbols.
        for(auto s : kv.second)
        {
            //create multiple threads to change symbols simultaneously,
            //since the symbols and references don't overlap eachother, no need to consider access competing.
            futs.push_back(async([&buf, s]{return change_symbol(buf, s);}));
            if(futs.size() >= min(size_t(64), kv.second.size() - index))
            {
                for(auto& f : futs)
                {
                    ++index;
                    flag = f.get() || flag;
                    cout << "rename symbol " << index << "/" << kv.second.size() << endl;
                }
                futs.clear();
            }
        }

        if(flag)
            write_file(kv.first, buf);
    }
    cout << "end" << endl;
    return 0;
}
