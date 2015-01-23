#ifndef CA_H
#define CA_H

class CA
{
public:
    void HK_registation(std::string name, std::string key);
    /**
    *	function to verify new client and sign his public key
    * 	@string par name
    * 	@string public_key
    **/

    void HK_sign_public_key(std::string key);
    /**
    *	@brief function to sign  client´s public key by CA´s private key
    * 	@string public_key
    **/
}
#endif // CA_H
