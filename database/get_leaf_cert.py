"""
Leaf Certificate Fetcher and Inspector for Misconfigured HTTPS Servers.

This module retrieves **only the leaf certificate** (the server's own certificate)
from a given hostname over TLS, without verifying the chain.

Some servers are misconfigured and fail to send the full certificate chain
during the TLS handshake. While `curl` can still connect to these by relying
on system root certificates, higher-level libraries like `requests` may reject
the connection unless a full CA bundle is provided.
"""

import sys
import ssl
import socket
from cryptography import x509
from cryptography.x509 import Certificate
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization


def get_certificate(hostname):
    """
    Perform a raw TLS connection to fetch the server's leaf certificate.

    This bypasses certificate verification to ensure we can still retrieve
    certificates from misconfigured servers.

    Args:
        hostname (str): The domain to connect to (without scheme or port).

    Returns:
        x509.Certificate: Parsed certificate object, or None on failure.
    """
    context = ssl.create_default_context()
    context.check_hostname = False  # Don't verify host name
    context.verify_mode = ssl.CERT_NONE  # Don't verify certificate chain

    with socket.create_connection((hostname, 443)) as sock:
        with context.wrap_socket(sock, server_hostname=hostname) as ssock:
            cert_der = ssock.getpeercert(True)  # Get DER-encoded certificate
            if cert_der:
                return x509.load_der_x509_certificate(cert_der, default_backend())
    return None


def get_sha256_fingerprint(cert):
    """
    Compute SHA-256 fingerprint of a certificate.

    Args:
        cert (x509.Certificate): The certificate to hash.

    Returns:
        bytes: Raw SHA-256 digest.
    """
    cert_der = cert.public_bytes(serialization.Encoding.DER)
    digest = hashes.Hash(hashes.SHA256(), backend=default_backend())
    digest.update(cert_der)
    return digest.finalize()


def get_serial_number_hex(cert):
    """
    Return the certificate's serial number formatted as hexadecimal.

    Useful for identifying and verifying specific certs manually.

    Args:
        cert (x509.Certificate): The certificate to inspect.

    Returns:
        str: Colon-separated hex string (e.g., "01:AB:CD:EF").
    """
    serial_number_bytes = cert.serial_number.to_bytes(
        (cert.serial_number.bit_length() + 7) // 8, 'big'
    )
    return ':'.join(f'{b:02X}' for b in serial_number_bytes)


def print_certificate_details(cert):
    """
    Print human-readable information about the certificate.

    Includes fingerprint, serial number, subject and issuer details.

    Args:
        cert (x509.Certificate): The certificate to print.
    """
    fingerprint = get_sha256_fingerprint(cert)
    fingerprint_hex = ':'.join(f'{b:02X}' for b in fingerprint)
    serial_number_hex = get_serial_number_hex(cert)

    print(f"SHA-256 fingerprint: {fingerprint_hex}")
    print(f"Serial number: {serial_number_hex}")
    print(f"Subject: {cert.subject}")
    print(f"Issuer: {cert.issuer}")
    print()


def run(url):
    """
    Determine the target host from the URL, fetch and print its leaf certificate.

    Args:
        url (str): The full URL (must start with one of the known domains).

    Returns:
        bytes: The PEM-encoded certificate (as bytes), or exits on failure.
    """
    # Map URL to known hosts. These are the only ones we allow.
    if url.startswith("https://www.trle.net"):
        host = 'trle.net'
    elif url.startswith("https://trcustoms.org"):
        host = 'trcustoms.org'
    else:
        sys.exit(1)  # Reject unknown domains for safety

    certificate = get_certificate(host)
    if not certificate:
        print("Failed to retrieve certificate.")
        sys.exit(1)

    print_certificate_details(certificate)

    if not isinstance(certificate, Certificate):
        print("Invalid certificate object.")
        sys.exit(1)

    return certificate.public_bytes(encoding=serialization.Encoding.PEM)
