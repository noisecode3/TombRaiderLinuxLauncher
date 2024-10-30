"""This module get the certificate for "broken" servers that don't follow
   the standard handshake procedure, that is not sending the chain,
   curl can still connect to this server by specifying the leaf and curl will
   by default look for the chain in /etc/ssl/certs but requests module
   require a bundle so that one would have to compile this bundle into the chain"""

import sys
import ssl
import socket
from cryptography import x509
from cryptography.x509 import Certificate
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization

def get_certificate(hostname, port=443):
    """OpenSSL with TCP get the certificate"""
    context = ssl.create_default_context()
    # Disable certificate verification for the first connection
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE

    with socket.create_connection((hostname, port)) as sock:
        with context.wrap_socket(sock, server_hostname=hostname) as ssock:
            # Get certificate info
            cert_der = ssock.getpeercert(True)
            if cert_der:
                return x509.load_der_x509_certificate(cert_der, default_backend())
    return None

def get_sha256_fingerprint(cert):
    """Identify the sum, we might want verify the certificate"""
    cert_der = cert.public_bytes(serialization.Encoding.DER)
    digest = hashes.Hash(hashes.SHA256(), backend=default_backend())
    digest.update(cert_der)
    return digest.finalize()

def get_serial_number_hex(cert):
    """Identify the serial, this can be use to look for the certificate"""
    # Get the serial number in a byte format
    serial_number_bytes = cert.serial_number \
        .to_bytes((cert.serial_number.bit_length() + 7) // 8, 'big')
    # Format it as a hex string
    return ':'.join(f'{b:02X}' for b in serial_number_bytes)

def print_certificate_details(cert):
    """Log basic certificate information"""
    fingerprint = get_sha256_fingerprint(cert)
    fingerprint_hex = ':'.join(f'{b:02X}' for b in fingerprint)
    serial_number_hex = get_serial_number_hex(cert)

    print(f"SHA-256 fingerprint: {fingerprint_hex}")
    print(f"Serial number: {serial_number_hex}")
    print(f"Subject: {cert.subject}")
    print(f"Issuer: {cert.issuer}")
    print()


def run(url):
    if url.startswith("https://www.trle.net"):
        host = 'trle.net'
    elif url.startswith("https://trcustoms.org"):
        host = 'trcustoms.org'
    else:
        sys.exit(1)
    certificate = get_certificate(host)
    if not certificate:
        sys.exit(1)

    print_certificate_details(certificate)
    if not isinstance(certificate, Certificate):
        sys.exit(1)

    return certificate.public_bytes(encoding=serialization.Encoding.PEM)

'''
def validate_downloaded_key(id_number, expected_serial):
    """Validate the certificate in binary form with the cryptography module"""
    pem_key = get_response(f"https://crt.sh/?d={id_number}", 'application/pkix-cert')

    if not isinstance(pem_key, bytes):
        logging.error("Data type error, expected bytes got %s", type(pem_key))
        sys.exit(1)

    # Load the certificate
    certificate = x509.load_pem_x509_certificate(pem_key, default_backend())

    # Extract the serial number and convert it to hex (without leading '0x')
    hex_serial = f'{certificate.serial_number:x}'

    # Compare the serial numbers
    if hex_serial == expected_serial:
        print("The downloaded PEM key matches the expected serial number.")
    else:
        logging.error("Serial mismatch! Expected: %s, but got: %s", expected_serial, hex_serial)
        sys.exit(1)

    # Extract and validate the domain (Common Name)
    valid_domains = ["trle.net", "trcustoms.org", "data.trcustoms.org", "staging.trcustoms.org"]

    # Check the Common Name (CN) in the certificate subject
    comon_name = certificate.subject.get_attributes_for_oid(x509.NameOID.COMMON_NAME)[0].value
    if comon_name in valid_domains:
        print(f"Valid domain found in CN: {comon_name}")
    else:
        logging.error("Invalid domain in CN: %s", comon_name)
        sys.exit(1)

    # Extract the Subject Alternative Name (SAN) extension
    try:
        san_extension = certificate.extensions \
            .get_extension_for_oid(ExtensionOID.SUBJECT_ALTERNATIVE_NAME)

        # Extract all DNS names listed in the SAN extension
        dns_names = san_extension.value.get_values_for_type(x509.DNSName) # type: ignore

        print(f"DNS Names in SAN: {dns_names}")

        # Check if any of the DNS names match the valid domain list
        valid_domains = ["trle.net", "www.trle.net", "trcustoms.org", "*.trcustoms.org",
                         "data.trcustoms.org", 'staging.trcustoms.org']

        if all(domain in valid_domains for domain in dns_names):
            print(f"Valid domain found in SAN: {dns_names}")
        else:
            print(f"Invalid domain in SAN: {dns_names}")
            sys.exit(1)

    except x509.ExtensionNotFound:
        print("No Subject Alternative Name (SAN) extension found in the certificate.")

    pem_data = certificate.public_bytes(encoding=serialization.Encoding.PEM)
    return pem_data.decode('utf-8')
'''
