DOCKER_TAG = dev

.PHONY: all install clean

all:
	$(MAKE) -C ext all
	$(MAKE) -C client all
	$(MAKE) -C kafka all

install:
	$(MAKE) -C ext install
	$(MAKE) -C client install

clean:
	$(MAKE) -C ext clean
	$(MAKE) -C client clean
	$(MAKE) -C kafka clean

docker: docker-client docker-postgres docker-connect

docker-compose: docker
	docker-compose build

tmp:
	mkdir tmp

tmp/%.tar.gz: tmp docker-build
	docker run --rm bwbuild:$(DOCKER_TAG) cat /$*.tar.gz > $@

tmp/%: build/% tmp
	cp $< $@

docker-build:
	docker build -f build/Dockerfile.build -t bwbuild:$(DOCKER_TAG) .

docker-client: tmp/Dockerfile.client tmp/avro.tar.gz tmp/librdkafka.tar.gz tmp/bottledwater-bin.tar.gz tmp/bottledwater-docker-wrapper.sh
	docker build -f $< -t local-bottledwater:$(DOCKER_TAG) tmp

docker-postgres: tmp/Dockerfile.postgres tmp/bottledwater-ext.tar.gz tmp/avro.tar.gz tmp/replication-config.sh
	docker build -f $< -t local-postgres-bw:$(DOCKER_TAG) tmp

docker-connect: tmp/Dockerfile.connect tmp/bottledwater-lib.tar.gz
	docker build -f $< -t local-bottledwater-connect:$(DOCKER_TAG) tmp
