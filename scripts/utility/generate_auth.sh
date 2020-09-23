#!/bin/bash
echo "This script will create a private key, certificate and credentials file for use with a hub server."
echo "(You need to have openssl installed to successfully run it.)"
echo
echo "When generating the SSL certificate, you should enter the hostname and domain of the machine that"
echo "you intend to run the server on as the \"Common Name\""
echo ""
read -n1 -rsp "Press any key to start..."
openssl req -x509 -sha256 -nodes -days 3650 -newkey rsa:2048 -keyout jacktrip.key -out jacktrip.crt
echo ""
echo "Key created as jacktrip.key. Certificate created as jacktrip.crt."
echo ""
echo "Creating auth file. The username and password entered here can be used to access the server with"
echo "no time restrictions."
read -p "Username: " USERNAME
PASSWD=`openssl passwd -6`
echo "$USERNAME:$PASSWD:*" > auth
echo "Credentials written to auth"
