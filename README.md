# mod_fluentd

mod_fluentd is Fluentd log output module for Apache HTTPD Server.

## Build

```
% git clone --depth=1 https://github.com/kjdev/apache-mod-fluentd.git
% cd apache-mod-fluentd
% ./autogen.sh # OR autoreconf -i
% ./configure [OPTION]
% make
% make install
```

### Build options

apache path.

* --with-apxs=PATH
* --with-apr=PATH

## Configration

httpd.conf:

```
LoadModule fluentd_module modules/mod_fluentd.so
<IfModule fluentd_module>

    CustomLog "fluentd:tag" combined
    # Output:
    #   host: 127.0.0.1
    #   port: 5160 (UDP)
    #   out : {"tag":"tag","message":"127.0.0.1 - - .."}
    #         message to combined format

    ## Select Host/Port
    CustomLog "fluentd:host-port#192.168.122.123@8765" combined
    # Output:
    #   host: 192.168.122.123
    #   port: 8765 (UDP)
    #   out : {"tag":"host-port","message":"127.0.0.1 - - .."}
    #         message to combined format

    ## Append log message
    CustomLog 'fluentd:append {"out":"append"}' combined
    # Output:
    #   host: 127.0.0.1
    #   port: 5160 (UDP)
    #   out : {"tag":"append","out":"append","message":"127.0.0.1 - - .."}
    #         message to combined format

</IfModule>
```
