#pragma once

namespace mcview {

struct GLAttributes {
  GLAttributes(juce::OpenGLContext &openGLContext, juce::OpenGLShaderProgram &shader) {
    position.reset(createAttribute(openGLContext, shader, "position"));
    textureCoordIn.reset(createAttribute(openGLContext, shader, "textureCoordIn"));
  }

  void enable(juce::OpenGLContext &openGLContext) {
    using namespace juce::gl;

    if (position.get() != nullptr) {
      openGLContext.extensions.glVertexAttribPointer(position->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), nullptr);
      openGLContext.extensions.glEnableVertexAttribArray(position->attributeID);
    }

    if (textureCoordIn.get() != nullptr) {
      openGLContext.extensions.glVertexAttribPointer(textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)(sizeof(float) * 2));
      openGLContext.extensions.glEnableVertexAttribArray(textureCoordIn->attributeID);
    }
  }

  void disable(juce::OpenGLContext &openGLContext) {
    if (position.get() != nullptr)
      openGLContext.extensions.glDisableVertexAttribArray(position->attributeID);
    if (textureCoordIn.get() != nullptr)
      openGLContext.extensions.glDisableVertexAttribArray(textureCoordIn->attributeID);
  }

  std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, textureCoordIn;

private:
  static juce::OpenGLShaderProgram::Attribute *createAttribute(juce::OpenGLContext &openGLContext,
                                                               juce::OpenGLShaderProgram &shader,
                                                               const char *attributeName) {
    if (openGLContext.extensions.glGetAttribLocation(shader.getProgramID(), attributeName) < 0)
      return nullptr;

    return new juce::OpenGLShaderProgram::Attribute(shader, attributeName);
  }
};

} // namespace mcview
