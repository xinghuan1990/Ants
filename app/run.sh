cd /root/xingyuxuan/code_cilk_3/app/
echo "In code_cilk_3/app/"
echo "start components..."
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/twitter_ligra
echo "start BC..."
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/twitter_ligra
echo "start PageRank..."
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/twitter_ligra
echo "start MIS..."
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/twitter_ligra
echo "start Triangle..."
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/twitter_ligra

cd /root/xingyuxuan/ligra-master/apps/
echo "In ligra-master/apps"
echo "start components..."
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Components /root/xingyuxuan/dataset/twitter_ligra
echo "start BC..."
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./BC /root/xingyuxuan/dataset/twitter_ligra
echo "start PageRank..."
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./PageRank /root/xingyuxuan/dataset/twitter_ligra
echo "start MIS..."
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./MIS /root/xingyuxuan/dataset/twitter_ligra
echo "start Triangle..."
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/sk_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/gsh_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/enwiki_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/soc_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/hollywood_ligra
sync; echo 3 > /proc/sys/vm/drop_caches
./Triangle /root/xingyuxuan/dataset/twitter_ligra
