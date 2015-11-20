rm -rf output/build/pinController-1.0/
rm -rf output/build/sonar-1.0/
rm -rf output/build/helloWorld-666/
make
scp output/target/lib/modules/3.18.2/extra/sonar.ko root@10.0.0.2:/lib/modules/3.18.2/extra
scp output/target/bin/helloWorld root@10.0.0.2:/bin
