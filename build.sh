rm -rf output/build/pinController-1.0/
make
scp output/target/lib/modules/3.18.2/extra/pinController.ko root@10.0.0.2:/lib/modules/3.18.2/extra

