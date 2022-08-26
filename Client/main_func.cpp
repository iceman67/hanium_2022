#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

using namespace std;

#include "client.h"

int main(){
    if(!initClient()){
        cout << "Failed init socket!" << endl;
        return -1;
    }

	// while(1){
	// 	sleep(1);
	// }

    termClient();
}
