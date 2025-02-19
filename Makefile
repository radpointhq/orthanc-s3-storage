build-s3-plugin:
	docker build -t orthanc-s3-plugin -f scripts/docker-build-orthanc/Dockerfile .

build-orthanc: build-s3-plugin
	docker build -t orthanc-s3 -f tests/Dockerfile .

run: build-orthanc
	docker compose --env-file docker.env -f compose.yml up

stop:
	docker compose -f compose.yml stop

destroy:
	docker compose -f compose.yml down
