JOBS ?= 4
CATALOG ?= data/products.tsv

.PHONY: build run test coverage smoke clean help

build:
	cmake -S . -B build -DBUILD_TESTING=OFF
	cmake --build build --parallel "$(JOBS)"

run: build
	./build/cosmetics_bot "$(CATALOG)"

test:
	JOBS="$(JOBS)" ./scripts/test.sh

coverage:
	JOBS="$(JOBS)" ./scripts/test.sh --coverage

smoke:
	cmake -S . -B build -DBUILD_TESTING=OFF
	cmake --build build --target telegram_smoke --parallel "$(JOBS)"
	./build/telegram_smoke

clean:
	cmake -E remove_directory build
	cmake -E remove_directory build-tests
	cmake -E remove_directory build-coverage

help:
	@printf '%s\n' 'make build     собрать бота' 'make run       собрать и запустить бота' 'make test      запустить GoogleTest' 'make coverage  запустить тесты с отчётом покрытия' 'make smoke     проверить токен через Telegram getMe' 'make clean     удалить папки сборки'
