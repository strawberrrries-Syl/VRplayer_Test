# Panorama Video player `C++`
This project was realized by OpenCV, GLAD, GLFW, GLM, and FFmpeg. 
--
In this project, a panorama video player is generated. The main idea is listed below. You can see the `demo video` at https://drive.google.com/file/d/1JgR2DlJy7TDk4CfFzg9GMgJIfntZiyI-/view?usp=sharing

VR is popular nowadays. With panorama multimedia sources, I want to realize a player which can use them. Meanwhile, I’m interested in games and videos. So, the first thing that comes to my mind is realizing a VR player. 

System design is straightforward. 
> `model a sphere.`
> 
> `Render the video on it.`

## Sphere modeling
Used GLFW with GLAD to manage OpenGL. During the development, GLM was also used for vertex calculating. 
* Vertex calculating. I divide the whole sphere into 20 longitude parts and 50 latitude parts. Then generated all vertex as well as triangles.
* Draw all triangles and get the index of them. Then bound to buffers waiting for texture applied.

## Video processing
Videos we are watching needs to be decoded. I used FFmpeg to decode them.
* Decode frames of video into buffers for YUV format. 
* Render 3 pictures (Y, U, and V) on the model. Then pause for a while and refresh the window.
* Swap buffer, continue next loop

