#!/bin/bash
set -x
set -e

curl -L -O https://github.com/samtools/samtools/releases/download/1.9/samtools-1.9.tar.bz2
tar xvf samtools-1.9.tar.bz2
pushd samtools-1.9
./configure
make -j$(nproc)
popd

curl -O http://hgdownload.cse.ucsc.edu/goldenPath/hg19/encodeDCC/wgEncodeUwRepliSeq/wgEncodeUwRepliSeqK562G1AlnRep1.bam

PATH=samtools-1.9:${PATH}
samtools sort -m1M wgEncodeUwRepliSeqK562G1AlnRep1.bam -o wgEncodeUwRepliSeqK562G1AlnRep1.sorted.bam

rm samtools-1.9.tar.bz2
rm -rf samtools-1.9
rm -rf *.bam
