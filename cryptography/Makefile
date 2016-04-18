all:
	g++ -std=c++11 -g3 -ggdb -O0 lgtm_crypto.cpp lgtm_crypto_runner.cpp lgtm_file_utils.cpp -o lgtm_crypto_runner -L../../cryptopp -lcryptopp -static -lpthread
test:
	g++ -std=c++11 -g3 -ggdb -O0 -DTESTING -Wall lgtm_crypto.cpp lgtm_crypto_runner.cpp lgtm_file_utils.cpp lgtm_crypto_runner_test.cpp -o lgtm_crypto_runner_test -L../../cryptopp -lcryptopp -static -lpthread
clean-files:
	rm .lgtm-crypto-params-* .lgtm-facial-recognition-params-* .lgtm-received-facial-recognition-params* .lgtm-test-* 2>/dev/null
