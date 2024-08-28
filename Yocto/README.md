# Yocto image
https://www.codeinsideout.com/blog/yocto/raspberry-pi/

docker run -it --rm -v ${pwd}/build:/home/yocto/build yocto-dev-env

Could not mount any volum from host!!
Two bash files:
1. One for single module run
2. Whole image run


***
This shows the running container
`docker ps` 
To copy the build folder out of the docker
`docker cp container_id:/home/yocto/build ./build`
What does --rm do in the command?
How can you mound a local drive including the application to the docker container and re-run it?