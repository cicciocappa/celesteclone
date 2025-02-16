#gcc main.c glad/src/gl.c -o celeste -lglfw -lGL -lm

#gcc main.c glad/src/gl.c -o celeste -lglfw -lGL

gcc src/main.c glad/glad.c -lglfw -ldl -o celeste
