ENVIRONMENT := "esp32dev"

compiledb:
  pio run -t compiledb -e {{ENVIRONMENT}}

build: compiledb
  pio run -e {{ENVIRONMENT}}

upload:
  pio run -e {{ENVIRONMENT}} -t upload

test:
  pio test -e native -v

monitor:
  pio device monitor
