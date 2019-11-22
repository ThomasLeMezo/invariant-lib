rm `pwd`/$1/*.png

for file in `pwd`/$1/*
do
	export output=${file%\.*}.png
	echo $output
  	iperender -png -resolution 600 "$file" $output
done

echo "" > list_png.txt
for file in `pwd`/$1/*.png
do
	echo file \'$file\' >> list_png.txt
done

ffmpeg -y -r 30 -f concat -safe 0 -i "list_png.txt" -c:v libx264 -r 30 out.mp4