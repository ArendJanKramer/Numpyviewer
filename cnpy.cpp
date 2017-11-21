//Copyright (C) 2011  Carl Rogers
//Released under MIT License
//license available in LICENSE file, or at http://www.opensource.org/licenses/mit-license.php
//#define NDEBUG

#include"cnpy.h"
#include<complex>
#include<cstdlib>
#include<algorithm>
#include<cstring>
#include<iomanip>
#include<stdint.h>

char cnpy::BigEndianTest() {
    int x = 1;
    return (((char *)&x)[0]) ? '<' : '>';
}

char cnpy::map_type(const std::type_info& t)
{
    if(t == typeid(float) ) return 'f';
    if(t == typeid(double) ) return 'f';
    if(t == typeid(long double) ) return 'f';

    if(t == typeid(int) ) return 'i';
    if(t == typeid(char) ) return 'i';
    if(t == typeid(short) ) return 'i';
    if(t == typeid(long) ) return 'i';
    if(t == typeid(long long) ) return 'i';

    if(t == typeid(unsigned char) ) return 'u';
    if(t == typeid(unsigned short) ) return 'u';
    if(t == typeid(unsigned long) ) return 'u';
    if(t == typeid(unsigned long long) ) return 'u';
    if(t == typeid(unsigned int) ) return 'u';

    if(t == typeid(bool) ) return 'b';

    if(t == typeid(std::complex<float>) ) return 'c';
    if(t == typeid(std::complex<double>) ) return 'c';
    if(t == typeid(std::complex<long double>) ) return 'c';

    else return '?';
}

template<> std::vector<char>& cnpy::operator+=(std::vector<char>& lhs, const std::string rhs) {
    lhs.insert(lhs.end(),rhs.begin(),rhs.end());
    return lhs;
}

template<> std::vector<char>& cnpy::operator+=(std::vector<char>& lhs, const char* rhs) {
    //write in little endian
    size_t len = strlen(rhs);
    lhs.reserve(len);
    for(size_t byte = 0; byte < len; byte++) {
        lhs.push_back(rhs[byte]);
    }
    return lhs;
}

void cnpy::parse_npy_header(char* buffer,size_t& word_size, std::vector<size_t>& shape, bool& fortran_order) {
    std::string magic_string(buffer,6);
    //uint8_t major_version = *reinterpret_cast<uint8_t*>(buffer+6);
    //uint8_t minor_version = *reinterpret_cast<uint8_t*>(buffer+7);
    uint16_t header_len = *reinterpret_cast<uint16_t*>(buffer+8);
    std::string header(buffer+9,header_len);

    int loc1, loc2;

    //fortran order
    loc1 = header.find("fortran_order")+16;
    fortran_order = (header.substr(loc1,4) == "True" ? true : false);

    //shape
    loc1 = header.find("(");
    loc2 = header.find(")");
    std::string str_shape = header.substr(loc1+1,loc2-loc1-1);
    size_t ndims;
    if(str_shape[str_shape.size()-1] == ',') ndims = 1;
    else ndims = std::count(str_shape.begin(),str_shape.end(),',')+1;
    shape.resize(ndims);
    for(size_t i = 0;i < ndims;i++) {
        loc1 = str_shape.find(",");
        shape[i] = atoi(str_shape.substr(0,loc1).c_str());
        str_shape = str_shape.substr(loc1+1);
    }

    //endian, word size, data type
    //byte order code | stands for not applicable. 
    //not sure when this applies except for byte array
    loc1 = header.find("descr")+9;
    bool littleEndian = (header[loc1] == '<' || header[loc1] == '|' ? true : false);
    assert(littleEndian);

    //char type = header[loc1+1];
    //assert(type == map_type(T));

    std::string str_ws = header.substr(loc1+2);
    loc2 = str_ws.find("'");
    word_size = atoi(str_ws.substr(0,loc2).c_str());
}

void cnpy::parse_npy_header(FILE* fp, size_t& word_size, std::vector<size_t>& shape, bool& fortran_order) {  
    char buffer[256];
    size_t res = fread(buffer,sizeof(char),11,fp);       
    if(res != 11){
        printf("parse_npy_header: failed res");
        fflush(stdout);

       // throw std::runtime_error("parse_npy_header: failed fread");
    }
    std::string header = fgets(buffer,256,fp);
    assert(header[header.size()-1] == '\n');

    int loc1, loc2;

    //fortran order
    loc1 = header.find("fortran_order")+16;
    fortran_order = (header.substr(loc1,4) == "True" ? true : false);

    //shape
    loc1 = header.find("(");
    loc2 = header.find(")");
    std::string str_shape = header.substr(loc1+1,loc2-loc1-1);
    size_t ndims;
    if(str_shape[str_shape.size()-1] == ',') ndims = 1;
    else ndims = std::count(str_shape.begin(),str_shape.end(),',')+1;
    shape.resize(ndims);
    for(size_t i = 0;i < ndims;i++) {
        loc1 = str_shape.find(",");
        shape[i] = atoi(str_shape.substr(0,loc1).c_str());
        str_shape = str_shape.substr(loc1+1);
    }

    //endian, word size, data type
    //byte order code | stands for not applicable. 
    //not sure when this applies except for byte array
    loc1 = header.find("descr")+9;
    assert(header[loc1] == '<' || header[loc1] == '|');


    //char type = header[loc1+1];
    //assert(type == map_type(T));

    std::string str_ws = header.substr(loc1+2);
    loc2 = str_ws.find("'");
    word_size = atoi(str_ws.substr(0,loc2).c_str());
}


cnpy::NpyArray load_the_npy_file(FILE* fp) {
    std::vector<size_t> shape;
    size_t word_size;
    bool fortran_order;
    cnpy::parse_npy_header(fp,word_size,shape,fortran_order);

    cnpy::NpyArray arr(shape, word_size, fortran_order);
    size_t nread = fread(arr.data<char>(),1,arr.num_bytes(),fp);
    if(nread != arr.num_bytes()){
        printf("parse_npy_header: failed nread ");
        fflush(stdout);
       // throw std::runtime_error("load_the_npy_file: failed fread");
    }
    return arr;
}



cnpy::NpyArray cnpy::npy_load(std::string fname) {

    FILE* fp = fopen(fname.c_str(), "rb");

    if(!fp) {
        printf("\nnpy_load: Error! Unable to open file %s!\n",fname.c_str());
        fflush(stdout);

        abort();  
    }

    NpyArray arr = load_the_npy_file(fp);

    fclose(fp);
    return arr;
}



