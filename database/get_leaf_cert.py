"""This module get the certificate for "broken" servers that don't follow
   the standard handshake procedure, that is not sending the chain,
   curl can still connect to this server by specifying the leaf and curl will
   by default look for the chain in /etc/ssl/certs but requests module
   require a bundle so that one would have to compile this bundle into the chain"""

import sys
import ssl
import socket
from cryptography import x509
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
    print_certificate_details(certificate)
    return certificate # this is bytes data type
