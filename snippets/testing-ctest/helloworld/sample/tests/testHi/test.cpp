#include <iostream>

using namespace std;

int 
main(int argc, char **argv)
{
    cout << "Here goes: " << argv[0] << endl;
    cout << "Argc = " << argc << endl;
    return (argc > 3);
}
