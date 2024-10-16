run-virtual-can:
	./virtual-can.sh

send-can:
	cansend vcan0 123#00FFAA5501020304

create-docker-compiler:
	docker run -itd --name=godot-extension-compiler -v ./:/$(USER) -u $(USER) godot-arm64-compiler

compile-extension-editor:
	docker start godot-extension-compiler
	docker exec -it --workdir /$(USER)/ godot-extension-compiler scons platform=linux

compile-extension-arm64:
	docker start godot-extension-compiler
	docker exec -it --workdir /$(USER)/ godot-extension-compiler scons platform=linux target=template_release arch=arm64 CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++