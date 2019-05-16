#pragma once

#define ResourceDirectory    std::string("Data")
#define ImagesDirectory      std::string(ResourceDirectory + "/" + "Images")
#define SoundsDirectory      std::string(ResourceDirectory + "/" + "Sounds")
#define ShadersDirectory     std::string(ResourceDirectory + "/" + "Shaders")

#define ImageFilename        std::string("image.png")
#define ShaderFilename       std::string("shader.frag")
#define DummyFilename        std::string("dummy.txt")

#define ImageResourcePath    std::string("Images" + "/" + ImageFilename)
#define ShaderResourcePath   std::string("Shaders" + "/" + ShaderFilename)
#define DummyResourcePath    std::string(DummyFilename)

#define ImageFilePath        std::string(ImagesDirectory + "/" + "image.png")
#define ShaderFilePath       std::string(ShadersDirectory + "/" + "shader.frag")
#define DummyFilePath        std::string(ResourceDirectory + "/" + "dummy.txt")

#define MaximumTimeoutWaitMS long long(5000)