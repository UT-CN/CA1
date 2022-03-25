#include <iostream>
#include <json/value.h>
#include <fstream>

using namespace std;

int main(){
    ifstream config_file("config.json", ifstream::binary);
    config_file >> config;

    cout << config; //This will print the entire json object.

    // //The following lines will let you access the indexed objects.
    // cout<<people["Anna"]; //Prints the value for "Anna"
    // cout<<people["ben"]; //Prints the value for "Ben"
    // cout<<people["Anna"]["profession"]; //Prints the value corresponding to "profession" in the json for "Anna"

    // cout<<people["profession"]; //NULL! There is no element with key "profession". Hence a new empty element will be created.


    return 0;
}