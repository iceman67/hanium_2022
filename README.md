# Logger_Verifier + CLient, Server

Ignore .vscode
Client: folder where client.cpp exist

Logger_Verifier: folder where Logger and Verifier code exist

Server: folder where Server code Exist

# to Complie
1. Client, Server
```
make clean && make 
```
2. Logger_Verifier
```
make clean && make Logger Verifier
```
3. PRIVATE, PUBLIC KEY TEST
```
FIRST DOWNLOAD CRYPTO++ in your PI at home directory
git clone https://github.com/weidai11/cryptopp.git
```
```
cd cryptopp
```
```
make
```
```
sudo make install
```
```
come back to Logger_Verifier folder
make sign_verify
```
