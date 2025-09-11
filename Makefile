MAX_LINE_LENGTH = 120

SYSTEM := $(shell uname -s)

black:
	@black python/ --line-length $(MAX_LINE_LENGTH)
	@black extrapolation.py --line-length $(MAX_LINE_LENGTH)

isort:
	@isort python/ --line-length $(MAX_LINE_LENGTH) --multi-line 3 --profile black
	@isort extrapolation.py --line-length $(MAX_LINE_LENGTH) --multi-line 3 --profile black

flake8:
	@flake8 python/ --exclude venv/,testing/memory_profiling/ --max-line-length $(MAX_LINE_LENGTH) --ignore=E203,W503
	@flake8 extrapolation.py --exclude venv/,testing/memory_profiling/ --max-line-length $(MAX_LINE_LENGTH) --ignore=E203,W503

pyformat:
	@make black
	@make isort
	@make flake8

cppformat:
	@clang-format -i --style Mozilla cpp/board_game_permutations_with_border.cpp
	@clang-format -i --style Mozilla cpp/board_game_permutations_no_border.cpp
	@clang-format -i --style Mozilla cpp/board_game_permutations_utils.cpp
	@clang-format -i --style Mozilla cpp/board_game_permutations_utils.h

compile:
ifeq ($(SYSTEM), Darwin)
	@echo 'Compiling on MacOS'
	@g++ -O3 -march=native -ffast-math -std=c++17 cpp/board_game_permutations_utils.cpp cpp/board_game_permutations_with_border.cpp -o cpp/board_game_permutations_with_border
	#@g++ -O3 -march=native -ffast-math -std=c++17 cpp/board_game_permutations_utils.cpp cpp/board_game_permutations_no_border.cpp -o cpp/board_game_permutations_no_border
else ifeq ($(SYSTEM), Linux)
	@echo "Compiling on Linux"
	@clang++ -O3 -march=native -stdlib=libc++ -ffast-math -std=c++17 cpp/board_game_permutations_utils.cpp cpp/board_game_permutations_with_border.cpp -o cpp/board_game_permutations_with_border
	#@clang++ -O3 -march=native -stdlib=libc++ -ffast-math -std=c++17 cpp/board_game_permutations_utils.cpp cpp/board_game_permutations_no_border.cpp -o cpp/board_game_permutations_no_border
else
	@echo "Unsupported operating system $(SYSTEM)"
endif

run:
	@./cpp/board_game_permutations_with_border -s 3 -n 1
