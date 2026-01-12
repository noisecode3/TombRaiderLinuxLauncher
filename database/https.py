"""Get a request response for https only with curl."""
import os
import sys
import time
import json
import socket
import logging
import tempfile
import hashlib
from urllib.parse import urlparse
from io import BytesIO
import pycurl
from tqdm import tqdm

import get_leaf_cert
import data_factory


class AcquireLock:
    """
    Create a TCP socket to ensure a single instance.

    This class creates a TCP socket that binds to a specific port. If an instance
    of this class is already running, it will log an error and exit.

    Example usage:

    ```python
    lock = AcquireLock()
    try:
        # Your application logic here
        pass  # Replace with actual code
    finally:
        lock.release_lock()  # Ensure the lock is released when done
    ```

    Attributes:
        lock (socket.socket): The TCP socket used to enforce the single instance.
        To protect the server from user error.
    """

    def __init__(self):
        """Set the lock, so that it don't accidentally span too many requests."""
        self.lock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            # Bind to localhost and a specific port
            self.lock.bind(('127.0.0.1', 55234))
        except socket.error:
            logging.error("Another instance is already running")
            sys.exit(1)

    def release_lock(self):
        """Release the socket and close the connection."""
        if self.lock:
            self.lock.close()
            self.lock = None

    def is_locked(self):
        """Check if the lock is active."""
        return self.lock is not None


class RequestHandler:
    """Handle HTTPS requests with retry and certificate handling."""

    def __init__(self):
        """Set default values."""
        self.misconfigured_server = False
        self.leaf_cert = None

    def validate_url(self, url):
        """Limit to used domains."""
        allowed_domains = (
            "https://www.trle.net/",
            "https://trle.net/",
            "https://trcustoms.org/",
            "https://data.trcustoms.org/",
            "https://www.trlevel.de/"
        )

        if any(url.startswith(domain) for domain in allowed_domains):
            return  # URL is valid

        logging.error("Invalid URL domain: %s", url)
        sys.exit(1)

    def head(self, url):
        """Take the curl object to head state, with redirect."""
        curl = pycurl.Curl()
        response = ""
        temp_cert_path = None
        try:
            buffer = BytesIO()
            curl.setopt(pycurl.HEADERFUNCTION, buffer.write)
            curl.setopt(pycurl.NOBODY, True)
            curl.setopt(pycurl.URL, url)
            curl.setopt(pycurl.FOLLOWLOCATION, False)

            if self.misconfigured_server:
                if not self.leaf_cert:
                    sys.exit(1)
                temp_cert_path = REQUEST_HANDLER.set_leaf(curl)

            curl.perform()
            response = buffer.getvalue().decode('utf-8')

        except pycurl.error:
            print("Error performing request:", pycurl.error)
        finally:
            curl.close()

            if temp_cert_path and os.path.exists(temp_cert_path):
                os.remove(temp_cert_path)

        return response

    def validate_data_type(self, content_type):
        """Limit to used data types."""
        valid_content_types = {
            'application/json',
            'application/pkix-cert',
            'application/zip',
            'image/jpeg',
            'image/png',
            'text/html',
            'head'  # this is not MIME
        }

        if content_type not in valid_content_types:
            logging.error("Invalid content type: %s", content_type)
            sys.exit(1)

    def set_leaf(self, curl):
        """Write the certificate to a temporary file manually."""
        if self.leaf_cert is None:
            raise ValueError("Leaf certificate is None and cannot be written to a file.")

        # Use 'with' to ensure the temporary file is closed properly
        with tempfile.NamedTemporaryFile(delete=False) as temp_cert_file:
            temp_cert_file.write(self.leaf_cert)  # self.leaf_cert must be of type 'bytes'
            temp_cert_file.flush()  # Flush to ensure data is written to disk
            temp_cert_path = temp_cert_file.name  # Get the name of the temporary file

        # Set CAINFO to use the temporary certificate file
        curl.setopt(pycurl.CAINFO, temp_cert_path)
        return temp_cert_path

    def get_leaf(self, url):
        """Check if we need to grab the first certificate."""
        if not self.misconfigured_server:
            self.leaf_cert = get_leaf_cert.run(url)
        if not isinstance(self.leaf_cert, bytes):
            sys.exit(1)

        if self.leaf_cert:
            self.misconfigured_server = True
            logging.info("Leaf certificate retrieved and applied.")
        else:
            logging.error("Failed to retrieve leaf certificate. Exiting.")
            sys.exit(1)

    def setup_before_get_response(self, url, content_type):
        """Validate known URL and content type."""
        if not url:
            logging.error("Faild setup_before_get_response, url is None. Exiting.")
            sys.exit(1)
        self.validate_url(url)
        self.validate_data_type(content_type)

        if url.startswith("https://www.trle.net/") and not self.misconfigured_server:
            self.get_leaf(url)

    def get_response(self, url, content_type):
        """Handle all https requests."""
        self.setup_before_get_response(url, content_type)

        if url.startswith("https://trcustoms.org"):
            self.misconfigured_server = False

        if url.startswith("https://www.trlevel.de"):
            self.misconfigured_server = False

        if content_type == 'application/zip':
            return DOWNLOADER.download_file(url)

        if content_type == 'head':
            return self.head(url)

        max_retries = 3
        retries = 0
        curl = None
        headers = None
        response_buffer = None
        temp_cert_path = None

        while retries < max_retries:
            try:
                response_buffer = BytesIO()
                headers_buffer = BytesIO()
                curl = pycurl.Curl()  # pylint: disable=no-member
                curl.setopt(pycurl.URL, url)
                curl.setopt(pycurl.WRITEHEADER, headers_buffer)
                curl.setopt(pycurl.WRITEDATA, response_buffer)

                if self.misconfigured_server:
                    if not self.leaf_cert:
                        sys.exit(1)
                    temp_cert_path = self.set_leaf(curl)
                    curl.setopt(pycurl.SSL_VERIFYPEER, 1)
                    curl.setopt(pycurl.SSL_VERIFYHOST, 2)
                    pinned_key = "sha256//CUN/bgQXn7cHaZejJcWCFhq/OIaGoxya1ojtnyRRdGA="
                    curl.setopt(pycurl.PINNEDPUBLICKEY, pinned_key)

                headers_list = [
                    'User-Agent: Wget/1.21.1 (linux-gnu)',
                    'Accept: */*',
                ]
                curl.setopt(pycurl.HTTPHEADER, headers_list)
                curl.perform()

                response_code = curl.getinfo(pycurl.RESPONSE_CODE)

                if response_code != 200:
                    retries += 1
                    time.sleep(3)
                    logging.warning("Retrying... Response code: %s", response_code)
                    curl.close()
                    continue

                headers = headers_buffer.getvalue().decode('utf-8')
                break

            except pycurl.error as curl_error:
                # if curl_error.args[0] == 60:  # SSL certificate error
                logging.error("Request failed: %s", curl_error)
                retries += 1
                if retries >= max_retries:
                    logging.error("Max retries reached. Exiting.")
                    sys.exit(1)

            finally:
                if temp_cert_path and os.path.exists(temp_cert_path):
                    os.remove(temp_cert_path)

        return self.close_response(curl, headers, response_buffer, content_type)

    def close_response(self, curl, headers, response_buffer, content_type):
        """Pack response and close curl."""
        if curl is None:
            logging.error("No curl instance")
            sys.exit(1)

        if headers is None:
            logging.error("No headers received")
            sys.exit(1)

        if response_buffer is None:
            logging.error("No response received")
            sys.exit(1)

        # Extract Content-Type from the headers
        response_content_type = self.extract_content_type(headers)
        if response_content_type == content_type:
            response = self.pack_response_buffer(content_type, response_buffer)
            curl.close()
            return response
        logging.error("Unexpected content type: %s, expected %s",
                      response_content_type, content_type)
        sys.exit(1)

    def pack_response_buffer(self, content_type, response_buffer):
        """Validate and return the response based on content type."""
        if content_type == 'text/html':
            raw_data = response_buffer.getvalue()
            for encoding in ['utf-8', 'windows-1252', 'utf-16', 'utf-32']:
                try:
                    return raw_data.decode(encoding)
                except UnicodeDecodeError:
                    continue
            logging.error("No known encoding")
            sys.exit(1)
        if content_type == 'application/json':
            return json.loads(response_buffer.getvalue().decode('utf-8'))
        if content_type in ['image/jpeg', 'image/png']:
            return response_buffer.getvalue()
        if content_type == 'application/pkix-cert':
            return response_buffer.getvalue()
        logging.error("Unsupported content type: %s", content_type)
        return None

    def extract_content_type(self, headers):
        """Read the header lines to look for content-type."""
        for header in headers.splitlines():
            if header.lower().startswith('content-type:'):
                return header.split(':', 1)[1].split(';')[0].strip()
        logging.error("Could not extract content type from header: %s", headers)
        return None


class Downloader:
    """Zip file downloader to be used in RequestHandler."""

    def __init__(self):
        """Set default values."""
        self.buffer = BytesIO()
        self.status = 0
        self.progress_bar = None

    def write_callback(self, data):
        """Write the downloaded data."""
        self.buffer.write(data)
        return len(data)

    def progress_callback(self, total_to_download, downloaded, total_to_upload, uploaded):
        """Report download progress.

        Args:
            total_to_download (int): Total size of the file to download.
            downloaded (int): Number of bytes downloaded so far.
            total_to_upload (int): Total size of the file to upload (not used).
            uploaded (int): Number of bytes uploaded (not used).
        """
        _ = uploaded  # Explicitly ignore 'uploaded'
        _ = total_to_upload  # Explicitly ignore 'total_to_upload'
        if total_to_download > 0:
            if self.progress_bar is None:
                # Initialize the progress bar if it's not set
                self.progress_bar = tqdm(total=total_to_download,
                                         unit='B',
                                         unit_scale=True,
                                         unit_divisor=1024,
                                         desc="Downloading")

            self.progress_bar.update(downloaded - self.progress_bar.n)  # Update the progress bar
            self.progress_bar.total = total_to_download
        return 0  # Returning 0 means to continue

    def download_file(self, url):
        """
        Download a file from the trle URL and stores its contents in a buffer.

        This method utilizes the `pycurl` library to perform the download, providing
        a progress bar for user feedback. It handles server misconfigurations,
        follows redirects, and calculates the MD5 checksum of the downloaded file.

        Parameters:
        ----------
        url : str
            The URL of the file to download. Must be a valid URL.

        Raises:
        -------
        SystemExit
            Exits the program if the server is misconfigured and no leaf certificate is available.

        Exceptions:
        ------------
        pycurl.error
            Raised if an error occurs during the download process.

        Returns:
        --------
        dict
            Returns a dictionary containing details of the downloaded file, including:
            - 'size': Size of the file in MiB (mebibytes).
            - 'url': The effective URL from which the file was downloaded.
            - 'name': The name of the file.
            - 'md5': The MD5 checksum of the downloaded content.

        Notes:
        ------
        - The progress bar is displayed using the `tqdm` library to indicate the download status.
        - The method checks the HTTP response code after the download to ensure success (HTTP 200).
        - Temporary files created for certificate handling are cleaned up after the download.
        """
        curl = pycurl.Curl()
        temp_cert_path = None
        zip_file = data_factory.make_zip_file()  # Initialize the zip_file dictionary

        try:
            # Get file size for the progress bar
            curl.setopt(pycurl.NOBODY, True)  # Disable body output for this request
            curl.setopt(pycurl.URL, url)
            curl.setopt(pycurl.FOLLOWLOCATION, True)  # Follow redirects

            if REQUEST_HANDLER.misconfigured_server:
                if not REQUEST_HANDLER.leaf_cert:
                    sys.exit(1)
                temp_cert_path = REQUEST_HANDLER.set_leaf(curl)

            curl.perform()  # Header only

            # Get header info
            total_size = curl.getinfo(pycurl.CONTENT_LENGTH_DOWNLOAD)
            zip_file['size'] = round(total_size / (1024 * 1024), 2)  # Size in MiB
            zip_file['url'] = curl.getinfo(pycurl.EFFECTIVE_URL)
            zip_file['name'] = os.path.basename(urlparse(zip_file['url']).path)

            # Main buffer
            curl.setopt(pycurl.NOBODY, False)  # Re-enable body output
            curl.setopt(pycurl.WRITEFUNCTION, self.write_callback)
            curl.setopt(pycurl.WRITEDATA, self.buffer)

            # Enable progress meter
            self.progress_bar = tqdm(total=total_size,
                                     unit='B',
                                     unit_scale=True,
                                     unit_divisor=1024,
                                     desc="Downloading")

            curl.setopt(pycurl.NOPROGRESS, False)
            curl.setopt(pycurl.XFERINFOFUNCTION, self.progress_callback)

            # Perform the download
            curl.perform()

            # Check for errors
            http_code = curl.getinfo(pycurl.RESPONSE_CODE)
            if http_code != 200:
                self.status = 1
                print(f"Error: HTTP response code {http_code}")
                return {}  # Return an empty dict on error

            self.status = 0

            # Finalize MD5 checksum
            md5_hash = hashlib.md5(usedforsecurity=False)
            self.buffer.seek(0)  # Reset buffer pointer
            md5_hash.update(self.buffer.getvalue())
            zip_file['md5'] = md5_hash.hexdigest()

        except pycurl.error as e:
            self.status = 1
            print(f"Error: {e}")
            return {}  # Return an empty dict on error

        finally:
            if self.progress_bar:
                self.progress_bar.close()
            curl.close()
            if temp_cert_path:
                if os.path.exists(temp_cert_path):
                    os.remove(temp_cert_path)

        return zip_file  # Always return the zip_file dictionary




ACQUIRE_LOCK = AcquireLock()
REQUEST_HANDLER = RequestHandler()
DOWNLOADER = Downloader()


def get(url, content_type):
    """
    Get server response from TRLE or Trcustom hosts.

    content_type:
        'application/json'
        'application/pkix-cert'
        'application/zip'
        'image/jpeg'
        'image/png'
        'text/html'
        'head'

    url must start with:
        "https://www.trle.net/"
        "https://trcustoms.org/"
        "https://data.trcustoms.org/"
    """
    return REQUEST_HANDLER.get_response(url, content_type)


def release_lock():
    """Release lock for this instance."""
    ACQUIRE_LOCK.release_lock()


def is_locked():
    """Lock this instance."""
    ACQUIRE_LOCK.is_locked()
