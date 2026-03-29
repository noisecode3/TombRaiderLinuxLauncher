#!/bin/bash
openssl s_client -connect trle.net:443 -servername trle.net < /dev/null | openssl x509 -pubkey -noout > pubkey.pem
openssl pkey -pubin -inform PEM -in pubkey.pem -outform DER | sha256sum | xxd -r -p | base64
