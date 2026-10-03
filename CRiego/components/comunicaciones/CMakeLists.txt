idf_component_register(
    SRCS
        "comunicaciones.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        mqtt
        cjson
        sensores
        freertos
        actuadores

    EMBED_TXTFILES
        "root_ca.pem"
        "device_cert.pem"
        "device_key.pem"
)