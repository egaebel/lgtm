all:
	g++ -std=c++11 -g3 -ggdb -O0 -I . lgtm_crypto.cpp lgtm_crypto_runner.cpp lgtm_crypto_runner_test.cpp ../../cryptopp/libcryptopp.a -o lgtm_crypto_runner_test -lcryptopp -static -lpthread
clean:
	rm lgtm_crypto.o lgtm_crypto_runner.o lgtm_crypto_runner_test
