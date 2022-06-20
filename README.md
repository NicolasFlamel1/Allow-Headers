# Allow Headers

### Description
Nginx module that allows blocking all response HTTP headers except for those that are specified.

### Installing
Run the following commands to build and install this module.
```
wget "https://github.com/NicolasFlamel1/Allow-Headers/archive/refs/heads/master.zip"
unzip "./master.zip"
wget "https://nginx.org/download/nginx-$(nginx -v 2>&1 | awk '{print $3}'  | awk -F'/' '{print $2}').tar.gz"
tar -xf "./nginx-$(nginx -v 2>&1 | awk '{print $3}'  | awk -F'/' '{print $2}').tar.gz"
cd "./nginx-$(nginx -v 2>&1 | awk '{print $3}'  | awk -F'/' '{print $2}')"
echo $(nginx -V 2>&1 >/dev/null | grep -oP '(?<=^configure arguments: ).*?(?= --add-dynamic-module)') --add-dynamic-module="../Allow-Headers-master" | xargs "./configure"
make modules
sudo mv "./objs/ngx_http_allow_headers_module.so" "/usr/share/nginx/modules/"
```

Add the following line to the `top-level` context in your Nginx configuration file, `/etc/nginx/nginx.conf`, to enable this module.
```
load_module modules/ngx_http_allow_headers_module.so;
```

### Usage
This module provides the following directives that can be used in a `location` context.
1. `allow_headers`: This directive accepts a parameter with the value `on` or `off` which enables or disables blocking all response HTTP headers respectively. 
2. `allow_header`: This directive accepts a parameter with the case-sensitive value of a response HTTP header that won't be blocked when `allow_headers` is `on`.

For example, the following demonstrates how to allow only the `Content-Type`, `Transfer-Encoding`, `Content-Length`, and `Content-Encoding` response HTTP headers for requests made to `/`.
```
location / {
	allow_headers on;
	allow_header Content-Type;
	allow_header Transfer-Encoding;
	allow_header Content-Length;
	allow_header Content-Encoding;
}
```
