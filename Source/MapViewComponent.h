#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "RegionTextureCache.h"
#include <map>

class MapViewComponent : public Component, private OpenGLRenderer
{
public:
    MapViewComponent();
    ~MapViewComponent();

    void paint(Graphics &g) override;
    
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void mouseMagnify(MouseEvent const& event, float scaleFactor) override;
    void mouseWheelMove(MouseEvent const& event, MouseWheelDetails const& wheel) override;
    void mouseDrag(MouseEvent const& event) override;
    void mouseDown(MouseEvent const& event) override;
    void mouseMove(MouseEvent const& event) override;

    void setRegionsDirectory(File directory);

private:
    void updateShader();

    Point<float> getMapCoordinateFromView(Point<float> p) const;
    
    void magnify(Point<float> p, float rate);

    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            texture.reset(createUniform(openGLContext, shader, "texture"));
            fade.reset(createUniform(openGLContext, shader, "fade"));
            heightmap.reset(createUniform(openGLContext, shader, "heightmap"));
            blocksPerPixel.reset(createUniform(openGLContext, shader, "blocksPerPixel"));
            width.reset(createUniform(openGLContext, shader, "width"));
            height.reset(createUniform(openGLContext, shader, "height"));
            Xr.reset(createUniform(openGLContext, shader, "Xr"));
            Zr.reset(createUniform(openGLContext, shader, "Zr"));
            Cx.reset(createUniform(openGLContext, shader, "Cx"));
            Cz.reset(createUniform(openGLContext, shader, "Cz"));
        }

        std::unique_ptr<OpenGLShaderProgram::Uniform> texture, fade, heightmap, blocksPerPixel, width, height, Xr, Zr, Cx, Cz;

    private:
        static OpenGLShaderProgram::Uniform* createUniform(OpenGLContext& openGLContext,
                                                           OpenGLShaderProgram& shader,
                                                           const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation(shader.getProgramID(), uniformName) < 0) {
                return nullptr;
            }

            return new OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };

    struct Vertex
    {
        float position[2];
        float texCoord[2];
    };


    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            position.reset(createAttribute(openGLContext, shader, "position"));
            textureCoordIn.reset(createAttribute(openGLContext, shader, "textureCoordIn"));
        }

        void enable (OpenGLContext& openGLContext)
        {
            if (position.get() != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 2));
                openGLContext.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable (OpenGLContext& openGLContext)
        {
            if (position.get() != nullptr)        openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (textureCoordIn.get() != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<OpenGLShaderProgram::Attribute> position, textureCoordIn;

    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
                                                                OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    struct Buffer {
        GLuint vBuffer;
        GLuint iBuffer;
        
        static GLsizei const kNumPoints = 4;
    };

private:
    OpenGLContext fOpenGLContext;
    File fRegionsDirectory;
    std::map<Region, std::shared_ptr<RegionTextureCache>> fTextures;
    std::unique_ptr<OpenGLShaderProgram> fShader;
    std::unique_ptr<Uniforms> fUniforms;
    std::unique_ptr<Attributes> fAttributes;
    std::unique_ptr<Buffer> fBuffer;

    struct LookAt {
        float fX;
        float fZ;
        float fBlocksPerPixel;
    };
    Atomic<LookAt> fLookAt;

    static float const kMaxScale;
    static float const kMinScale;

    Point<float> fCenterWhenDragStart;

    Point<float> fMouse;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MapViewComponent)
};
