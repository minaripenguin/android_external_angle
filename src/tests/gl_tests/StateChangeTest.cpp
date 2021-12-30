//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StateChangeTest:
//   Specifically designed for an ANGLE implementation of GL, these tests validate that
//   ANGLE's dirty bits systems don't get confused by certain sequences of state changes.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

#include <thread>

using namespace angle;

namespace
{

class StateChangeTest : public ANGLETest
{
  protected:
    StateChangeTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Enable the no error extension to avoid syncing the FBO state on validation.
        setNoErrorEnabled(true);
    }

    void testSetUp() override
    {
        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(2, mTextures.data());
        glGenRenderbuffers(1, &mRenderbuffer);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        if (!mTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(mTextures.size()), mTextures.data());
            mTextures.clear();
        }

        glDeleteRenderbuffers(1, &mRenderbuffer);
    }

    GLuint mFramebuffer           = 0;
    GLuint mRenderbuffer          = 0;
    std::vector<GLuint> mTextures = {0, 0};
};

class StateChangeTestES3 : public StateChangeTest
{
  protected:
    StateChangeTestES3() {}
};

// Ensure that CopyTexImage2D syncs framebuffer changes.
TEST_P(StateChangeTest, CopyTexImage2DSync)
{
    // TODO(geofflang): Fix on Linux AMD drivers (http://anglebug.com/1291)
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Init first texture to red
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Init second texture to green
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Copy in the red texture to the green one.
    // CopyTexImage should sync the framebuffer attachment change.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 16, 16, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    ASSERT_GL_NO_ERROR();
}

// Ensure that CopyTexSubImage2D syncs framebuffer changes.
TEST_P(StateChangeTest, CopyTexSubImage2DSync)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Init first texture to red
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Init second texture to green
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Copy in the red texture to the green one.
    // CopyTexImage should sync the framebuffer attachment change.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 16, 16);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    ASSERT_GL_NO_ERROR();
}

// Test that Framebuffer completeness caching works when color attachments change.
TEST_P(StateChangeTest, FramebufferIncompleteColorAttachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture at color attachment 0 to be non-color-renderable.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 16, 16, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that caching works when color attachments change with TexStorage.
TEST_P(StateChangeTest, FramebufferIncompleteWithTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_storage"));

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture at color attachment 0 to be non-color-renderable.
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_ALPHA8_EXT, 16, 16);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that caching works when color attachments change with CompressedTexImage2D.
TEST_P(StateChangeTestES3, FramebufferIncompleteWithCompressedTex)
{
    // ETC texture formats are not supported on Mac OpenGL. http://anglebug.com/3853
    ANGLE_SKIP_TEST_IF(IsOSX() && IsDesktopOpenGL());

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture at color attachment 0 to be non-color-renderable.
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, 16, 16, 0, 128, nullptr);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that caching works when color attachments are deleted.
TEST_P(StateChangeTestES3, FramebufferIncompleteWhenAttachmentDeleted)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Delete the texture at color attachment 0.
    glDeleteTextures(1, &mTextures[0]);
    mTextures[0] = 0;
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that Framebuffer completeness caching works when depth attachments change.
TEST_P(StateChangeTest, FramebufferIncompleteDepthAttachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 16, 16);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture at color attachment 0 to be non-depth-renderable.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that Framebuffer completeness caching works when stencil attachments change.
TEST_P(StateChangeTest, FramebufferIncompleteStencilAttachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 16, 16);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture at the stencil attachment to be non-stencil-renderable.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that Framebuffer completeness caching works when depth-stencil attachments change.
TEST_P(StateChangeTestES3, FramebufferIncompleteDepthStencilAttachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 16, 16);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Change the texture the depth-stencil attachment to be non-depth-stencil-renderable.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that enabling GL_SAMPLE_ALPHA_TO_COVERAGE doesn't generate errors.
TEST_P(StateChangeTest, AlphaToCoverageEnable)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // We don't actually care that this does anything, just that it can be enabled without causing
    // an error.
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    glUseProgram(greenProgram);
    drawQuad(greenProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

const char kSimpleAttributeVS[] = R"(attribute vec2 position;
attribute vec4 testAttrib;
varying vec4 testVarying;
void main()
{
    gl_Position = vec4(position, 0, 1);
    testVarying = testAttrib;
})";

const char kSimpleAttributeFS[] = R"(precision mediump float;
varying vec4 testVarying;
void main()
{
    gl_FragColor = testVarying;
})";

// Tests that using a buffered attribute, then disabling it and using current value, works.
TEST_P(StateChangeTest, DisablingBufferedVertexAttribute)
{
    ANGLE_GL_PROGRAM(program, kSimpleAttributeVS, kSimpleAttributeFS);
    glUseProgram(program);
    GLint attribLoc   = glGetAttribLocation(program, "testAttrib");
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, attribLoc);
    ASSERT_NE(-1, positionLoc);

    // Set up the buffered attribute.
    std::vector<GLColor> red(6, GLColor::red);
    GLBuffer attribBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, attribBuffer);
    glBufferData(GL_ARRAY_BUFFER, red.size() * sizeof(GLColor), red.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(attribLoc);
    glVertexAttribPointer(attribLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

    // Also set the current value to green now.
    glVertexAttrib4f(attribLoc, 0.0f, 1.0f, 0.0f, 1.0f);

    // Set up the position attribute as well.
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw with the buffered attribute. Verify red.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with the disabled "current value attribute". Verify green.
    glDisableVertexAttribArray(attribLoc);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify setting buffer data on the disabled buffer doesn't change anything.
    std::vector<GLColor> blue(128, GLColor::blue);
    glBindBuffer(GL_ARRAY_BUFFER, attribBuffer);
    glBufferData(GL_ARRAY_BUFFER, blue.size() * sizeof(GLColor), blue.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that setting value for a subset of default attributes doesn't affect others.
TEST_P(StateChangeTest, SetCurrentAttribute)
{
    constexpr char kVS[] = R"(attribute vec4 position;
attribute mat4 testAttrib;  // Note that this generates 4 attributes
varying vec4 testVarying;
void main (void)
{
    gl_Position = position;

    testVarying = position.y < 0.0 ?
                    position.x < 0.0 ? testAttrib[0] : testAttrib[1] :
                    position.x < 0.0 ? testAttrib[2] : testAttrib[3];
})";

    ANGLE_GL_PROGRAM(program, kVS, kSimpleAttributeFS);
    glUseProgram(program);
    GLint attribLoc   = glGetAttribLocation(program, "testAttrib");
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, attribLoc);
    ASSERT_NE(-1, positionLoc);

    // Set the current value of two of the test attributes, while leaving the other two as default.
    glVertexAttrib4f(attribLoc + 1, 0.0f, 1.0f, 0.0f, 1.0f);
    glVertexAttrib4f(attribLoc + 2, 0.0f, 0.0f, 1.0f, 1.0f);

    // Set up the position attribute.
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw and verify the four section in the output:
    //
    //  +---------------+
    //  | Black | Green |
    //  +-------+-------+
    //  | Blue  | Black |
    //  +---------------+
    //
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const int w                            = getWindowWidth();
    const int h                            = getWindowHeight();
    constexpr unsigned int kPixelTolerance = 5u;
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::black, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, GLColor::blue, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, GLColor::black, kPixelTolerance);
}

// Tests that drawing with transform feedback paused, then lines without transform feedback works
// without Vulkan validation errors.
TEST_P(StateChangeTestES3, DrawPausedXfbThenNonXfbLines)
{
    // glTransformFeedbackVaryings for program2 returns GL_INVALID_OPERATION on both Linux and
    // windows.  http://anglebug.com/4265
    ANGLE_SKIP_TEST_IF(IsIntel() && IsOpenGL());
    // http://anglebug.com/5388
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program1, essl1_shaders::vs::Simple(),
                                        essl1_shaders::fs::Blue(), tfVaryings, GL_SEPARATE_ATTRIBS);

    GLBuffer xfbBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 6 * sizeof(float[4]), nullptr, GL_STATIC_DRAW);

    GLTransformFeedback xfb;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);

    glUseProgram(program1);
    glBeginTransformFeedback(GL_TRIANGLES);
    glPauseTransformFeedback();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ANGLE_GL_PROGRAM(program2, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program2);
    glDrawArrays(GL_LINES, 0, 6);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();
}

// Tests that vertex attribute value is preserved across context switches.
TEST_P(StateChangeTest, MultiContextVertexAttribute)
{
    EGLWindow *window   = getEGLWindow();
    EGLDisplay display  = window->getDisplay();
    EGLConfig config    = window->getConfig();
    EGLSurface surface  = window->getSurface();
    EGLContext context1 = window->getContext();

    // Set up program in primary context
    ANGLE_GL_PROGRAM(program1, kSimpleAttributeVS, kSimpleAttributeFS);
    glUseProgram(program1);
    GLint attribLoc   = glGetAttribLocation(program1, "testAttrib");
    GLint positionLoc = glGetAttribLocation(program1, "position");
    ASSERT_NE(-1, attribLoc);
    ASSERT_NE(-1, positionLoc);

    // Set up the position attribute in primary context
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Set primary context attribute to green and draw quad
    glVertexAttrib4f(attribLoc, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Set up and switch to secondary context
    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };
    EGLContext context2 = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    ASSERT_NE(context2, EGL_NO_CONTEXT);
    eglMakeCurrent(display, surface, surface, context2);

    // Set up program in secondary context
    ANGLE_GL_PROGRAM(program2, kSimpleAttributeVS, kSimpleAttributeFS);
    glUseProgram(program2);
    ASSERT_EQ(attribLoc, glGetAttribLocation(program2, "testAttrib"));
    ASSERT_EQ(positionLoc, glGetAttribLocation(program2, "position"));

    // Set up the position attribute in secondary context
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // attribLoc current value should be default - (0,0,0,1)
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Restore primary context
    eglMakeCurrent(display, surface, surface, context1);
    // ReadPixels to ensure context is switched
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Switch to secondary context second time
    eglMakeCurrent(display, surface, surface, context2);
    // Check that it still draws black
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Restore primary context second time
    eglMakeCurrent(display, surface, surface, context1);
    // Check if it still draws green
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clean up
    eglDestroyContext(display, context2);
}

// Ensure that CopyTexSubImage3D syncs framebuffer changes.
TEST_P(StateChangeTestES3, CopyTexSubImage3DSync)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Init first texture to red
    glBindTexture(GL_TEXTURE_3D, mTextures[0]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextures[0], 0, 0);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Init second texture to green
    glBindTexture(GL_TEXTURE_3D, mTextures[1]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextures[1], 0, 0);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Copy in the red texture to the green one.
    // CopyTexImage should sync the framebuffer attachment change.
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextures[0], 0, 0);
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 0, 0, 16, 16);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextures[1], 0, 0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    ASSERT_GL_NO_ERROR();
}

// Ensure that BlitFramebuffer syncs framebuffer changes.
TEST_P(StateChangeTestES3, BlitFramebufferSync)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsVulkan());
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Init first texture to red
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Init second texture to green
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Change to the red textures and blit.
    // BlitFramebuffer should sync the framebuffer attachment change.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0],
                           0);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    ASSERT_GL_NO_ERROR();
}

// Ensure that ReadBuffer and DrawBuffers sync framebuffer changes.
TEST_P(StateChangeTestES3, ReadBufferAndDrawBuffersSync)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Initialize two FBO attachments
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mTextures[1], 0);

    // Clear first attachment to red
    GLenum bufs1[] = {GL_COLOR_ATTACHMENT0, GL_NONE};
    glDrawBuffers(2, bufs1);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear second texture to green
    GLenum bufs2[] = {GL_NONE, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs2);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Verify first attachment is red and second is green
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    ASSERT_GL_NO_ERROR();
}

// Tests calling invalidate on incomplete framebuffers after switching attachments.
// Adapted partially from WebGL 2 test "renderbuffers/invalidate-framebuffer"
TEST_P(StateChangeTestES3, IncompleteRenderbufferAttachmentInvalidateSync)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsVulkan());
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    GLint samples = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, 1, &samples);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderbuffer);
    ASSERT_GL_NO_ERROR();

    // invalidate the framebuffer when the attachment is incomplete: no storage allocated to the
    // attached renderbuffer
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    ASSERT_GL_NO_ERROR();

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(samples), GL_RGBA8,
                                     getWindowWidth(), getWindowHeight());
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    GLRenderbuffer renderbuf;

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuf.get());
    ASSERT_GL_NO_ERROR();

    // invalidate the framebuffer when the attachment is incomplete: no storage allocated to the
    // attached renderbuffer
    // Note: the bug will only repro *without* a call to checkStatus before the invalidate.
    GLenum attachments2[] = {GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments2);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(samples),
                                     GL_DEPTH_COMPONENT16, getWindowWidth(), getWindowHeight());
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClear(GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
}

class StateChangeRenderTest : public StateChangeTest
{
  protected:
    StateChangeRenderTest() : mProgram(0), mRenderbuffer(0) {}

    void testSetUp() override
    {
        StateChangeTest::testSetUp();

        constexpr char kVS[] =
            "attribute vec2 position;\n"
            "void main() {\n"
            "    gl_Position = vec4(position, 0, 1);\n"
            "}";
        constexpr char kFS[] =
            "uniform highp vec4 uniformColor;\n"
            "void main() {\n"
            "    gl_FragColor = uniformColor;\n"
            "}";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        glGenRenderbuffers(1, &mRenderbuffer);
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteRenderbuffers(1, &mRenderbuffer);

        StateChangeTest::testTearDown();
    }

    void setUniformColor(const GLColor &color)
    {
        glUseProgram(mProgram);
        const Vector4 &normalizedColor = color.toNormalizedVector();
        GLint uniformLocation          = glGetUniformLocation(mProgram, "uniformColor");
        ASSERT_NE(-1, uniformLocation);
        glUniform4fv(uniformLocation, 1, normalizedColor.data());
    }

    GLuint mProgram;
    GLuint mRenderbuffer;
};

// Test that re-creating a currently attached texture works as expected.
TEST_P(StateChangeRenderTest, RecreateTexture)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Draw with red to the FBO.
    GLColor red(255, 0, 0, 255);
    setUniformColor(red);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, red);

    // Recreate the texture with green.
    GLColor green(0, 255, 0, 255);
    std::vector<GLColor> greenPixels(32 * 32, green);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenPixels.data());
    EXPECT_PIXEL_COLOR_EQ(0, 0, green);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Verify drawing blue gives blue. This covers the FBO sync with D3D dirty bits.
    GLColor blue(0, 0, 255, 255);
    setUniformColor(blue);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, blue);

    EXPECT_GL_NO_ERROR();
}

// Test that re-creating a currently attached renderbuffer works as expected.
TEST_P(StateChangeRenderTest, RecreateRenderbuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderbuffer);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw with red to the FBO.
    setUniformColor(GLColor::red);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Recreate the renderbuffer and clear to green.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 32, 32);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Verify drawing blue gives blue. This covers the FBO sync with D3D dirty bits.
    setUniformColor(GLColor::blue);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    EXPECT_GL_NO_ERROR();
}

// Test that recreating a texture with GenerateMipmaps signals the FBO is dirty.
TEST_P(StateChangeRenderTest, GenerateMipmap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Draw once to set the RenderTarget in D3D11
    GLColor red(255, 0, 0, 255);
    setUniformColor(red);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, red);

    // This may trigger the texture to be re-created internally.
    glGenerateMipmap(GL_TEXTURE_2D);

    // Explictly check FBO status sync in some versions of ANGLE no_error skips FBO checks.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Now ensure we don't have a stale render target.
    GLColor blue(0, 0, 255, 255);
    setUniformColor(blue);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, blue);

    EXPECT_GL_NO_ERROR();
}

// Tests that gl_DepthRange syncs correctly after a change.
TEST_P(StateChangeRenderTest, DepthRangeUpdates)
{
    constexpr char kFragCoordShader[] = R"(void main()
{
    if (gl_DepthRange.near == 0.2)
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
    else if (gl_DepthRange.near == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(0, 0, 1, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragCoordShader);
    glUseProgram(program);

    const auto &quadVertices = GetQuadVertices();

    ASSERT_EQ(0, glGetAttribLocation(program, essl1_shaders::PositionAttrib()));

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(quadVertices[0]),
                 quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0u);

    // First, clear.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw to left half viewport with a first depth range.
    glDepthRangef(0.2f, 1.0f);
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw to right half viewport with a second depth range.
    glDepthRangef(0.5f, 1.0f);
    glViewport(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Verify left half of the framebuffer is red and right half is green.
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight(), GLColor::red);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight(),
                         GLColor::green);
}

class StateChangeRenderTestES3 : public StateChangeRenderTest
{};

TEST_P(StateChangeRenderTestES3, InvalidateNonCurrentFramebuffer)
{
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // Draw with red to the FBO.
    GLColor red(255, 0, 0, 255);
    setUniformColor(red);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, red);

    // Go back to default framebuffer, draw green
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLColor green(0, 255, 0, 255);
    setUniformColor(green);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, green);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Invalidate color buffer of FBO
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    ASSERT_GL_NO_ERROR();

    // Verify drawing blue gives blue.
    GLColor blue(0, 0, 255, 255);
    setUniformColor(blue);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, blue);
}

// Tests that D3D11 dirty bit updates don't forget about BufferSubData attrib updates.
TEST_P(StateChangeTest, VertexBufferUpdatedAfterDraw)
{
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute vec4 color;\n"
        "varying vec4 outcolor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    outcolor = color;\n"
        "}";
    constexpr char kFS[] =
        "varying mediump vec4 outcolor;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = outcolor;\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer colorBuf;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuf);
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    // Fill with green.
    std::vector<GLColor> colorData(6, GLColor::green);
    glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(GLColor), colorData.data(),
                 GL_STATIC_DRAW);

    // Draw, expect green.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Update buffer with red.
    std::fill(colorData.begin(), colorData.end(), GLColor::red);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colorData.size() * sizeof(GLColor), colorData.data());

    // Draw, expect red.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Tests that drawing after flush without any state change works.
TEST_P(StateChangeTestES3, DrawAfterFlushWithNoStateChange)
{
    // Draw (0.125, 0.25, 0.5, 0.5) once, using additive blend
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);

    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    GLint positionLocation = glGetAttribLocation(drawColor, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    // Setup VAO
    const auto &quadVertices = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 6, quadVertices.data(), GL_STATIC_DRAW);

    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // Clear and draw
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUniform4f(colorUniformLocation, 0.125f, 0.25f, 0.5f, 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Make sure the work is submitted.
    glFinish();

    // Draw again with no state change
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Make sure the pixels have the correct colors.
    const int h = getWindowHeight() - 1;
    const int w = getWindowWidth() - 1;
    const GLColor kExpected(63, 127, 255, 255);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, kExpected, 1);
}

// Test that switching VAOs keeps the disabled "current value" attributes up-to-date.
TEST_P(StateChangeTestES3, VertexArrayObjectAndDisabledAttributes)
{
    constexpr char kSingleVS[] = "attribute vec4 position; void main() { gl_Position = position; }";
    constexpr char kSingleFS[] = "void main() { gl_FragColor = vec4(1, 0, 0, 1); }";
    ANGLE_GL_PROGRAM(singleProgram, kSingleVS, kSingleFS);

    constexpr char kDualVS[] =
        "#version 300 es\n"
        "in vec4 position;\n"
        "in vec4 color;\n"
        "out vec4 varyColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "    varyColor = color;\n"
        "}";
    constexpr char kDualFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 varyColor;\n"
        "out vec4 colorOut;\n"
        "void main()\n"
        "{\n"
        "    colorOut = varyColor;\n"
        "}";

    ANGLE_GL_PROGRAM(dualProgram, kDualVS, kDualFS);

    // Force consistent attribute locations
    constexpr GLint positionLocation = 0;
    constexpr GLint colorLocation    = 1;

    glBindAttribLocation(singleProgram, positionLocation, "position");
    glBindAttribLocation(dualProgram, positionLocation, "position");
    glBindAttribLocation(dualProgram, colorLocation, "color");

    {
        glLinkProgram(singleProgram);
        GLint linkStatus;
        glGetProgramiv(singleProgram, GL_LINK_STATUS, &linkStatus);
        ASSERT_NE(linkStatus, 0);
    }

    {
        glLinkProgram(dualProgram);
        GLint linkStatus;
        glGetProgramiv(dualProgram, GL_LINK_STATUS, &linkStatus);
        ASSERT_NE(linkStatus, 0);
    }

    glUseProgram(singleProgram);

    // Initialize position vertex buffer.
    const auto &quadVertices = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 6, quadVertices.data(), GL_STATIC_DRAW);

    // Initialize a VAO. Draw with single program.
    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Should draw red.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with a green buffer attribute, without the VAO.
    glBindVertexArray(0);
    glUseProgram(dualProgram);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    std::vector<GLColor> greenColors(6, GLColor::green);
    GLBuffer greenBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, greenBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * 6, greenColors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(colorLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Re-bind VAO and try to draw with different program, without changing state.
    // Should draw black since current value is not initialized.
    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

const char kSamplerMetadataVertexShader0[] = R"(#version 300 es
precision mediump float;
out vec4 color;
uniform sampler2D texture;
void main()
{
    vec2 size = vec2(textureSize(texture, 0));
    color = size.x != 0.0 ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 0.0);
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

const char kSamplerMetadataVertexShader1[] = R"(#version 300 es
precision mediump float;
out vec4 color;
uniform sampler2D texture1;
uniform sampler2D texture2;
void main()
{
    vec2 size1 = vec2(textureSize(texture1, 0));
    vec2 size2 = vec2(textureSize(texture2, 0));
    color = size1.x * size2.x != 0.0 ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 0.0);
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";

const char kSamplerMetadataFragmentShader[] = R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 result;
void main()
{
    result = color;
})";

// Tests that changing an active program invalidates the sampler metadata properly.
TEST_P(StateChangeTestES3, SamplerMetadataUpdateOnSetProgram)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    // TODO(anglebug.com/5491) Appears as though there's something wrong with textureSize on iOS
    // unrelated to switching programs.
    ANGLE_SKIP_TEST_IF(IsIOS() && IsOpenGLES());
    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);

    // Create a simple framebuffer.
    GLTexture texture1, texture2;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Create 2 shader programs differing only in the number of active samplers.
    ANGLE_GL_PROGRAM(program1, kSamplerMetadataVertexShader0, kSamplerMetadataFragmentShader);
    glUseProgram(program1);
    glUniform1i(glGetUniformLocation(program1, "texture"), 0);
    ANGLE_GL_PROGRAM(program2, kSamplerMetadataVertexShader1, kSamplerMetadataFragmentShader);
    glUseProgram(program2);
    glUniform1i(glGetUniformLocation(program2, "texture1"), 0);
    glUniform1i(glGetUniformLocation(program2, "texture2"), 0);

    // Draw a solid green color to the framebuffer.
    glUseProgram(program1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Test that our first program is good.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Bind a different program that uses more samplers.
    // Draw another quad that depends on the sampler metadata.
    glUseProgram(program2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Flush via ReadPixels and check that it's still green.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Tests that redefining Buffer storage syncs with the Transform Feedback object.
TEST_P(StateChangeTestES3, RedefineTransformFeedbackBuffer)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsVulkan());
    // Create the most simple program possible - simple a passthrough for a float attribute.
    constexpr char kVertexShader[] = R"(#version 300 es
in float valueIn;
out float valueOut;
void main()
{
    gl_Position = vec4(0, 0, 0, 0);
    valueOut = valueIn;
})";

    constexpr char kFragmentShader[] = R"(#version 300 es
out mediump float unused;
void main()
{
    unused = 1.0;
})";

    std::vector<std::string> tfVaryings = {"valueOut"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, kVertexShader, kFragmentShader, tfVaryings,
                                        GL_SEPARATE_ATTRIBS);
    glUseProgram(program);

    GLint attribLoc = glGetAttribLocation(program, "valueIn");
    ASSERT_NE(-1, attribLoc);

    // Disable rasterization - we're not interested in the framebuffer.
    glEnable(GL_RASTERIZER_DISCARD);

    // Initialize a float vertex buffer with 1.0.
    std::vector<GLfloat> data1(16, 1.0);
    GLsizei size1 = static_cast<GLsizei>(sizeof(GLfloat) * data1.size());

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, size1, data1.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attribLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(attribLoc);

    ASSERT_GL_NO_ERROR();

    // Initialize a same-sized XFB buffer.
    GLBuffer xfbBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, size1, nullptr, GL_STATIC_DRAW);

    // Draw with XFB enabled.
    GLTransformFeedback xfb;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 16);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    // Verify the XFB stage caught the 1.0 attribute values.
    void *mapped1     = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, size1, GL_MAP_READ_BIT);
    GLfloat *asFloat1 = reinterpret_cast<GLfloat *>(mapped1);
    std::vector<GLfloat> actualData1(asFloat1, asFloat1 + data1.size());
    EXPECT_EQ(data1, actualData1);
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    // Now, reinitialize the XFB buffer to a larger size, and draw with 2.0.
    std::vector<GLfloat> data2(128, 2.0);
    const GLsizei size2 = static_cast<GLsizei>(sizeof(GLfloat) * data2.size());
    glBufferData(GL_ARRAY_BUFFER, size2, data2.data(), GL_STATIC_DRAW);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, size2, nullptr, GL_STATIC_DRAW);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 128);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    // Verify the XFB stage caught the 2.0 attribute values.
    void *mapped2     = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, size2, GL_MAP_READ_BIT);
    GLfloat *asFloat2 = reinterpret_cast<GLfloat *>(mapped2);
    std::vector<GLfloat> actualData2(asFloat2, asFloat2 + data2.size());
    EXPECT_EQ(data2, actualData2);
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
}

// Simple state change tests for line loop drawing. There is some very specific handling of line
// line loops in Vulkan and we need to test switching between drawElements and drawArrays calls to
// validate every edge cases.
class LineLoopStateChangeTest : public StateChangeTest
{
  protected:
    LineLoopStateChangeTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void validateSquareAndHourglass() const
    {
        ASSERT_GL_NO_ERROR();

        int quarterWidth  = getWindowWidth() / 4;
        int quarterHeight = getWindowHeight() / 4;

        // Bottom left
        EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight, GLColor::blue);

        // Top left
        EXPECT_PIXEL_COLOR_EQ(quarterWidth, (quarterHeight * 3), GLColor::blue);

        // Top right
        // The last pixel isn't filled on a line loop so we check the pixel right before.
        EXPECT_PIXEL_COLOR_EQ((quarterWidth * 3), (quarterHeight * 3) - 1, GLColor::blue);

        // dead center to validate the hourglass.
        EXPECT_PIXEL_COLOR_EQ((quarterWidth * 2), quarterHeight * 2, GLColor::blue);

        // Verify line is closed between the 2 last vertices
        EXPECT_PIXEL_COLOR_EQ((quarterWidth * 2), quarterHeight, GLColor::blue);
    }
};

// Draw an hourglass with a drawElements call followed by a square with drawArrays.
TEST_P(LineLoopStateChangeTest, DrawElementsThenDrawArrays)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    // We expect to draw a square with these 4 vertices with a drawArray call.
    std::vector<Vector3> vertices;
    CreatePixelCenterWindowCoords({{8, 8}, {8, 24}, {24, 24}, {24, 8}}, getWindowWidth(),
                                  getWindowHeight(), &vertices);

    // If we use these indices to draw however, we should be drawing an hourglass.
    auto indices = std::vector<GLushort>{0, 2, 1, 3};

    GLint mPositionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, mPositionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0],
                 GL_STATIC_DRAW);

    glVertexAttribPointer(mPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mPositionLocation);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, nullptr);  // hourglass
    glDrawArrays(GL_LINE_LOOP, 0, 4);                             // square
    glDisableVertexAttribArray(mPositionLocation);

    validateSquareAndHourglass();
}

// Draw line loop using a drawArrays followed by an hourglass with drawElements.
TEST_P(LineLoopStateChangeTest, DrawArraysThenDrawElements)
{
    // http://anglebug.com/2856: Seems to fail on older drivers and pass on newer.
    // Tested failing on 18.3.3 and passing on 18.9.2.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsVulkan() && IsWindows());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    // We expect to draw a square with these 4 vertices with a drawArray call.
    std::vector<Vector3> vertices;
    CreatePixelCenterWindowCoords({{8, 8}, {8, 24}, {24, 24}, {24, 8}}, getWindowWidth(),
                                  getWindowHeight(), &vertices);

    // If we use these indices to draw however, we should be drawing an hourglass.
    auto indices = std::vector<GLushort>{0, 2, 1, 3};

    GLint mPositionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, mPositionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0],
                 GL_STATIC_DRAW);

    glVertexAttribPointer(mPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mPositionLocation);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_LINE_LOOP, 0, 4);                             // square
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, nullptr);  // hourglass
    glDisableVertexAttribArray(mPositionLocation);

    validateSquareAndHourglass();
}

// Draw a triangle with a drawElements call and a non-zero offset and draw the same
// triangle with the same offset again followed by a line loop with drawElements.
TEST_P(LineLoopStateChangeTest, DrawElementsThenDrawElements)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw a triangle with the last three points on the bottom right,
    // draw with LineLoop, and then draw a triangle with the same non-zero offset.
    auto vertices = std::vector<Vector3>{
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    auto indices = std::vector<GLushort>{0, 1, 2, 1, 2, 3};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0],
                 GL_STATIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Draw a triangle with a non-zero offset on the bottom right.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void *)(3 * sizeof(GLushort)));

    // Draw with LineLoop.
    glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_SHORT, nullptr);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void *)(3 * sizeof(GLushort)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate the top left point's color.
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::blue);

    // Validate the triangle is drawn on the bottom right.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::blue);

    // Validate the triangle is NOT on the top left part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::red);
}

// Simple state change tests, primarily focused on basic object lifetime and dependency management
// with back-ends that don't support that automatically (i.e. Vulkan).
class SimpleStateChangeTest : public ANGLETest
{
  protected:
    static constexpr int kWindowSize = 64;

    SimpleStateChangeTest()
    {
        setWindowWidth(kWindowSize);
        setWindowHeight(kWindowSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void simpleDrawWithBuffer(GLBuffer *buffer);
    void simpleDrawWithColor(const GLColor &color);

    using UpdateFunc = std::function<void(GLenum, GLTexture *, GLint, GLint, const GLColor &)>;
    void updateTextureBoundToFramebufferHelper(UpdateFunc updateFunc);
    void bindTextureToFbo(GLFramebuffer &fbo, GLTexture &texture);
    void drawToFboWithCulling(const GLenum frontFace, bool earlyFrontFaceDirty);
};

class SimpleStateChangeTestES3 : public SimpleStateChangeTest
{
  protected:
    void blendAndVerifyColor(const GLColor32F blendColor, const GLColor expectedColor)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        EXPECT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorUniformLocation =
            glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(colorUniformLocation, -1);

        glUniform4f(colorUniformLocation, blendColor.R, blendColor.G, blendColor.B, blendColor.A);
        EXPECT_GL_NO_ERROR();

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColor, 1);
    }
};

class SimpleStateChangeTestES31 : public SimpleStateChangeTestES3
{};

class SimpleStateChangeTestComputeES31 : public SimpleStateChangeTest
{
  protected:
    void testSetUp() override
    {
        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(1, &mTexture);

        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
        EXPECT_GL_NO_ERROR();

        constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=2, local_size_y=2) in;
layout (rgba8, binding = 0) readonly uniform highp image2D srcImage;
layout (rgba8, binding = 1) writeonly uniform highp image2D dstImage;
void main()
{
    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
               imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)));
})";

        mProgram = CompileComputeProgram(kCS);
        ASSERT_NE(mProgram, 0u);

        glBindImageTexture(1, mTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture,
                               0);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        if (mTexture != 0)
        {
            glDeleteTextures(1, &mTexture);
            mTexture = 0;
        }
        glDeleteProgram(mProgram);
    }

    GLuint mProgram;
    GLuint mFramebuffer = 0;
    GLuint mTexture     = 0;
};

class ImageES31PPO
{
  protected:
    ImageES31PPO() : mComputeProg(0), mPipeline(0) {}

    void bindProgramPipeline(const GLchar *computeString)
    {
        mComputeProg = glCreateShaderProgramv(GL_COMPUTE_SHADER, 1, &computeString);
        ASSERT_NE(mComputeProg, 0u);

        // Generate a program pipeline and attach the programs to their respective stages
        glGenProgramPipelines(1, &mPipeline);
        EXPECT_GL_NO_ERROR();
        glUseProgramStages(mPipeline, GL_COMPUTE_SHADER_BIT, mComputeProg);
        EXPECT_GL_NO_ERROR();
        glBindProgramPipeline(mPipeline);
        EXPECT_GL_NO_ERROR();
        glActiveShaderProgram(mPipeline, mComputeProg);
        EXPECT_GL_NO_ERROR();
    }

    GLuint mComputeProg;
    GLuint mPipeline;
};

class SimpleStateChangeTestComputeES31PPO : public ImageES31PPO, public SimpleStateChangeTest
{
  protected:
    SimpleStateChangeTestComputeES31PPO() : ImageES31PPO(), SimpleStateChangeTest() {}

    void testTearDown() override
    {
        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        if (mTexture != 0)
        {
            glDeleteTextures(1, &mTexture);
            mTexture = 0;
        }
        glDeleteProgramPipelines(1, &mPipeline);
    }

    GLuint mFramebuffer = 0;
    GLuint mTexture     = 0;
};

constexpr char kSimpleVertexShader[] = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}
)";

constexpr char kSimpleFragmentShader[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
)";

void SimpleStateChangeTest::simpleDrawWithBuffer(GLBuffer *buffer)
{
    ANGLE_GL_PROGRAM(program, kSimpleVertexShader, kSimpleFragmentShader);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
}

void SimpleStateChangeTest::simpleDrawWithColor(const GLColor &color)
{
    std::vector<GLColor> colors(6, color);
    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLColor), colors.data(), GL_STATIC_DRAW);
    simpleDrawWithBuffer(&colorBuffer);
}

// Test that we can do a drawElements call successfully after making a drawArrays call in the same
// frame.
TEST_P(SimpleStateChangeTest, DrawArraysThenDrawElements)
{
    // http://anglebug.com/4121
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGLES());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    // We expect to draw a triangle with the first 3 points to the left, then another triangle with
    // the last 3 vertices using a drawElements call.
    auto vertices = std::vector<Vector3>{{-1.0f, -1.0f, 0.0f},
                                         {-1.0f, 1.0f, 0.0f},
                                         {0.0f, 0.0f, 0.0f},
                                         {1.0f, 1.0f, 0.0f},
                                         {1.0f, -1.0f, 0.0f}};

    // If we use these indices to draw we'll be using the last 2 vertex only to draw.
    auto indices = std::vector<GLushort>{2, 3, 4};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0],
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    for (int i = 0; i < 10; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);                             // triangle to the left
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);  // triangle to the right
        glFinish();
    }
    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth = getWindowWidth() / 4;
    int halfHeight   = getWindowHeight() / 2;

    // Validate triangle to the left
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, halfHeight, GLColor::blue);

    // Validate triangle to the right
    EXPECT_PIXEL_COLOR_EQ((quarterWidth * 3), halfHeight, GLColor::blue);
}

// Draw a triangle with drawElements and a non-zero offset and draw the same
// triangle with the same offset followed by binding the same element buffer.
TEST_P(SimpleStateChangeTest, DrawElementsThenDrawElements)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw the triangle with the last three points on the bottom right, and
    // rebind the same element buffer and draw with the same indices.
    auto vertices = std::vector<Vector3>{
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    auto indices = std::vector<GLushort>{0, 1, 2, 1, 2, 3};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0],
                 GL_STATIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void *)(3 * sizeof(GLushort)));

    // Rebind the same element buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void *)(3 * sizeof(GLushort)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate the triangle is drawn on the bottom right.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::blue);

    // Validate the triangle is NOT on the top left part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::red);
}

// Draw a triangle with drawElements then change the index buffer and draw again.
TEST_P(SimpleStateChangeTest, DrawElementsThenDrawElementsNewIndexBuffer)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw the triangle with the last three points on the bottom right, and
    // rebind the same element buffer and draw with the same indices.
    auto vertices = std::vector<Vector3>{
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    auto indices8 = std::vector<GLubyte>{0, 1, 2, 1, 2, 3};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    GLBuffer indexBuffer8;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer8);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices8.size() * sizeof(GLubyte), &indices8[0],
                 GL_STATIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    auto indices2nd8 = std::vector<GLubyte>{2, 3, 0, 0, 1, 2};

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2nd8.size() * sizeof(GLubyte), &indices2nd8[0],
                 GL_STATIC_DRAW);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate the triangle is drawn on the bottom left.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::blue);

    // Validate the triangle is NOT on the top right part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::white);
}

// Draw a triangle with drawElements then change the indices and draw again.
TEST_P(SimpleStateChangeTest, DrawElementsThenDrawElementsNewIndices)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw the triangle with the last three points on the bottom right, and
    // rebind the same element buffer and draw with the same indices.
    std::vector<Vector3> vertices = {
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    std::vector<GLubyte> indices8 = {0, 1, 2, 2, 3, 0};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    GLBuffer indexBuffer8;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer8);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices8.size() * sizeof(GLubyte), &indices8[0],
                 GL_DYNAMIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    std::vector<GLubyte> newIndices8 = {2, 3, 0};

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, newIndices8.size() * sizeof(GLubyte),
                    &newIndices8[0]);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate the triangle is drawn on the bottom left.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::blue);

    // Validate the triangle is NOT on the top right part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::white);
}

// Draw a triangle with drawElements then change the indices and draw again.  Similar to
// DrawElementsThenDrawElementsNewIndices, but changes the whole index buffer (not just half).  This
// triggers a different path in the Vulkan backend based on the fact that the majority of the buffer
// is being updated.
TEST_P(SimpleStateChangeTest, DrawElementsThenDrawElementsWholeNewIndices)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw the triangle with the last three points on the bottom right, and
    // rebind the same element buffer and draw with the same indices.
    std::vector<Vector3> vertices = {
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    std::vector<GLubyte> indices8 = {0, 1, 2, 2, 3, 0};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    GLBuffer indexBuffer8;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer8);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices8.size() * sizeof(GLubyte), &indices8[0],
                 GL_DYNAMIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    std::vector<GLubyte> newIndices8 = {2, 3, 0, 0, 0, 0};

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, newIndices8.size() * sizeof(GLubyte),
                    &newIndices8[0]);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate the triangle is drawn on the bottom left.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::blue);

    // Validate the triangle is NOT on the top right part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::white);
}

// Draw a triangle with drawElements and a non-zero offset and draw the same
// triangle with the same offset followed by binding a USHORT element buffer.
TEST_P(SimpleStateChangeTest, DrawElementsUBYTEX2ThenDrawElementsUSHORT)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    glUseProgram(program);

    // Background Red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // We expect to draw the triangle with the last three points on the bottom right, and
    // rebind the same element buffer and draw with the same indices.
    auto vertices = std::vector<Vector3>{
        {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}};

    auto indices8 = std::vector<GLubyte>{0, 1, 2, 1, 2, 3};

    GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    GLBuffer indexBuffer8;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer8);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices8.size() * sizeof(GLubyte), &indices8[0],
                 GL_STATIC_DRAW);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    auto indices2nd8 = std::vector<GLubyte>{2, 3, 0, 0, 1, 2};
    GLBuffer indexBuffer2nd8;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2nd8);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2nd8.size() * sizeof(GLubyte), &indices2nd8[0],
                 GL_STATIC_DRAW);
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, (void *)(0 * sizeof(GLubyte)));

    // Bind the 16bit element buffer.
    auto indices16 = std::vector<GLushort>{0, 1, 3, 1, 2, 3};
    GLBuffer indexBuffer16;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer16);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices16.size() * sizeof(GLushort), &indices16[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer16);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draw the triangle again with the same offset.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (void *)(0 * sizeof(GLushort)));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    int quarterWidth  = getWindowWidth() / 4;
    int quarterHeight = getWindowHeight() / 4;

    // Validate green triangle is drawn on the bottom.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight, GLColor::green);

    // Validate white triangle is drawn on the right.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 3, quarterHeight * 2, GLColor::white);

    // Validate blue triangle is on the top left part.
    EXPECT_PIXEL_COLOR_EQ(quarterWidth * 2, quarterHeight * 3, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(quarterWidth, quarterHeight * 2, GLColor::blue);
}

// Draw a points use multiple unaligned vertex buffer with same data,
// verify all the rendering results are the same.
TEST_P(SimpleStateChangeTest, DrawRepeatUnalignedVboChange)
{
    // http://anglebug.com/4470
    ANGLE_SKIP_TEST_IF(isSwiftshader() && (IsWindows() || IsLinux()));

    const int kRepeat = 2;

    // set up VBO, colorVBO is unaligned
    GLBuffer positionBuffer;
    constexpr size_t posOffset = 0;
    const GLfloat posData[]    = {0.5f, 0.5f};
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(posData), posData, GL_STATIC_DRAW);

    GLBuffer colorBuffers[kRepeat];
    constexpr size_t colorOffset                = 1;
    const GLfloat colorData[]                   = {0.515f, 0.515f, 0.515f, 1.0f};
    constexpr size_t colorBufferSize            = colorOffset + sizeof(colorData);
    uint8_t colorDataUnaligned[colorBufferSize] = {0};
    memcpy(reinterpret_cast<void *>(colorDataUnaligned + colorOffset), colorData,
           sizeof(colorData));
    for (uint32_t i = 0; i < kRepeat; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffers[i]);
        glBufferData(GL_ARRAY_BUFFER, colorBufferSize, colorDataUnaligned, GL_STATIC_DRAW);
    }

    // set up frame buffer
    GLFramebuffer framebuffer;
    GLTexture framebufferTexture;
    bindTextureToFbo(framebuffer, framebufferTexture);

    // set up program
    ANGLE_GL_PROGRAM(program, kSimpleVertexShader, kSimpleFragmentShader);
    glUseProgram(program);
    GLuint colorAttrLocation = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorAttrLocation);
    GLuint posAttrLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(posAttrLocation);
    EXPECT_GL_NO_ERROR();

    // draw and get drawing results
    constexpr size_t kRenderSize = kWindowSize * kWindowSize;
    std::array<GLColor, kRenderSize> pixelBufs[kRepeat];

    for (uint32_t i = 0; i < kRepeat; i++)
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
        glVertexAttribPointer(posAttrLocation, 2, GL_FLOAT, GL_FALSE, 0,
                              reinterpret_cast<const void *>(posOffset));
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffers[i]);
        glVertexAttribPointer(colorAttrLocation, 4, GL_FLOAT, GL_FALSE, 0,
                              reinterpret_cast<const void *>(colorOffset));

        glDrawArrays(GL_POINTS, 0, 1);

        // read drawing results
        glReadPixels(0, 0, kWindowSize, kWindowSize, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelBufs[i].data());
        EXPECT_GL_NO_ERROR();
    }

    // verify something is drawn
    static_assert(kRepeat >= 2, "More than one repetition required");
    std::array<GLColor, kRenderSize> pixelAllBlack{0};
    EXPECT_NE(pixelBufs[0], pixelAllBlack);
    // verify drawing results are all identical
    for (uint32_t i = 1; i < kRepeat; i++)
    {
        EXPECT_EQ(pixelBufs[i - 1], pixelBufs[i]);
    }
}

// Handles deleting a Buffer when it's being used.
TEST_P(SimpleStateChangeTest, DeleteBufferInUse)
{
    std::vector<GLColor> colorData(6, GLColor::red);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * colorData.size(), colorData.data(),
                 GL_STATIC_DRAW);

    simpleDrawWithBuffer(&buffer);

    buffer.reset();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests that resizing a Buffer during a draw works as expected.
TEST_P(SimpleStateChangeTest, RedefineBufferInUse)
{
    std::vector<GLColor> redColorData(6, GLColor::red);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * redColorData.size(), redColorData.data(),
                 GL_STATIC_DRAW);

    // Trigger a pull from the buffer.
    simpleDrawWithBuffer(&buffer);

    // Redefine the buffer that's in-flight.
    std::vector<GLColor> greenColorData(1024, GLColor::green);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * greenColorData.size(), greenColorData.data(),
                 GL_STATIC_DRAW);

    // Trigger the flush and verify the first draw worked.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw again and verify the new data is correct.
    simpleDrawWithBuffer(&buffer);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests updating a buffer's contents while in use, without redefining it.
TEST_P(SimpleStateChangeTest, UpdateBufferInUse)
{
    std::vector<GLColor> redColorData(6, GLColor::red);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * redColorData.size(), redColorData.data(),
                 GL_STATIC_DRAW);

    // Trigger a pull from the buffer.
    simpleDrawWithBuffer(&buffer);

    // Update the buffer that's in-flight.
    std::vector<GLColor> greenColorData(6, GLColor::green);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLColor) * greenColorData.size(),
                    greenColorData.data());

    // Trigger the flush and verify the first draw worked.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw again and verify the new data is correct.
    simpleDrawWithBuffer(&buffer);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that deleting an in-flight Texture does not immediately delete the resource.
TEST_P(SimpleStateChangeTest, DeleteTextureInUse)
{
    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    draw2DTexturedQuad(0.5f, 1.0f, true);
    tex.reset();
    EXPECT_GL_NO_ERROR();

    int w = getWindowWidth() - 2;
    int h = getWindowHeight() - 2;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);
}

// Tests that modifying a texture parameter in-flight does not cause problems.
TEST_P(SimpleStateChangeTest, ChangeTextureFilterModeBetweenTwoDraws)
{
    std::array<GLColor, 4> colors = {
        {GLColor::black, GLColor::white, GLColor::black, GLColor::white}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw to the left side of the window only with NEAREST.
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, true);

    // Draw to the right side of the window only with LINEAR.
    glViewport(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    draw2DTexturedQuad(0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, getWindowWidth(), getWindowHeight());

    // The first half (left) should be only black followed by plain white.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 3, 0, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 4, 0, GLColor::white);

    // The second half (right) should be a gradient so we shouldn't find plain black/white in the
    // middle.
    EXPECT_NE(angle::ReadColor((getWindowWidth() / 4) * 3, 0), GLColor::black);
    EXPECT_NE(angle::ReadColor((getWindowWidth() / 4) * 3, 0), GLColor::white);
}

// Tests that bind the same texture all the time between different draw calls.
TEST_P(SimpleStateChangeTest, RebindTextureDrawAgain)
{
    GLuint program = get2DTexturedQuadProgram();
    glUseProgram(program);

    std::array<GLColor, 4> colors = {{GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan}};

    // Setup the texture
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup the vertex array to draw a quad.
    GLint positionLocation = glGetAttribLocation(program, "position");
    setupQuadVertexBuffer(1.0f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Bind again
    glBindTexture(GL_TEXTURE_2D, tex);
    ASSERT_GL_NO_ERROR();

    // Draw again, should still work.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Validate whole surface is filled with cyan.
    int h = getWindowHeight() - 1;
    int w = getWindowWidth() - 1;

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::cyan);
}

// Tests that we can draw with a texture, modify the texture with a texSubImage, and then draw again
// correctly.
TEST_P(SimpleStateChangeTest, DrawWithTextureTexSubImageThenDrawAgain)
{
    GLuint program = get2DTexturedQuadProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    std::array<GLColor, 4> colors    = {{GLColor::red, GLColor::red, GLColor::red, GLColor::red}};
    std::array<GLColor, 4> subColors = {
        {GLColor::green, GLColor::green, GLColor::green, GLColor::green}};

    // Setup the texture
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Setup the vertex array to draw a quad.
    GLint positionLocation = glGetAttribLocation(program, "position");
    setupQuadVertexBuffer(1.0f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Draw quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Update bottom-half of texture with green.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, subColors.data());
    ASSERT_GL_NO_ERROR();

    // Draw again, should still work.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Validate first half of the screen is red and the bottom is green.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 4 * 3, GLColor::red);
}

// Test that we can alternate between textures between different draws.
TEST_P(SimpleStateChangeTest, DrawTextureAThenTextureBThenTextureA)
{
    GLuint program = get2DTexturedQuadProgram();
    glUseProgram(program);

    std::array<GLColor, 4> colorsTex1 = {
        {GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan}};

    std::array<GLColor, 4> colorsTex2 = {
        {GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta}};

    // Setup the texture
    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorsTex1.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLTexture tex2;
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorsTex2.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup the vertex array to draw a quad.
    GLint positionLocation = glGetAttribLocation(program, "position");
    setupQuadVertexBuffer(1.0f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Draw quad
    glBindTexture(GL_TEXTURE_2D, tex1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Bind again, draw again
    glBindTexture(GL_TEXTURE_2D, tex2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Bind again, draw again
    glBindTexture(GL_TEXTURE_2D, tex1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Validate whole surface is filled with cyan.
    int h = getWindowHeight() - 1;
    int w = getWindowWidth() - 1;

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::cyan);
}

// Tests that redefining an in-flight Texture does not affect the in-flight resource.
TEST_P(SimpleStateChangeTest, RedefineTextureInUse)
{
    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw with the first texture.
    draw2DTexturedQuad(0.5f, 1.0f, true);

    // Redefine the in-flight texture.
    constexpr int kBigSize = 32;
    std::vector<GLColor> bigColors;
    for (int y = 0; y < kBigSize; ++y)
    {
        for (int x = 0; x < kBigSize; ++x)
        {
            bool xComp = x < kBigSize / 2;
            bool yComp = y < kBigSize / 2;
            if (yComp)
            {
                bigColors.push_back(xComp ? GLColor::cyan : GLColor::magenta);
            }
            else
            {
                bigColors.push_back(xComp ? GLColor::yellow : GLColor::white);
            }
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, bigColors.data());
    EXPECT_GL_NO_ERROR();

    // Verify the first draw had the correct data via ReadPixels.
    int w = getWindowWidth() - 2;
    int h = getWindowHeight() - 2;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);

    // Draw and verify with the redefined data.
    draw2DTexturedQuad(0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::white);
}

// Test updating a Texture's contents while in use by GL works as expected.
TEST_P(SimpleStateChangeTest, UpdateTextureInUse)
{
    std::array<GLColor, 4> rgby = {{GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    // Set up 2D quad resources.
    GLuint program = get2DTexturedQuadProgram();
    glUseProgram(program);
    ASSERT_EQ(0, glGetAttribLocation(program, "position"));

    const auto &quadVerts = GetQuadVertices();

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgby.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw RGBY to the Framebuffer. The texture is now in-use by GL.
    const int w  = getWindowWidth() - 2;
    const int h  = getWindowHeight() - 2;
    const int w2 = w >> 1;

    glViewport(0, 0, w2, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Update the texture to be YBGR, while the Texture is in-use. Should not affect the draw.
    std::array<GLColor, 4> ybgr = {{GLColor::yellow, GLColor::blue, GLColor::green, GLColor::red}};
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, ybgr.data());
    ASSERT_GL_NO_ERROR();

    // Draw again to the Framebuffer. The second draw call should use the updated YBGR data.
    glViewport(w2, 0, w2, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Check the Framebuffer. Both draws should have completed.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w2 - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w2 - 1, h - 1, GLColor::yellow);

    EXPECT_PIXEL_COLOR_EQ(w2 + 1, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w2 + 1, h - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

void SimpleStateChangeTest::updateTextureBoundToFramebufferHelper(UpdateFunc updateFunc)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    std::vector<GLColor> red(4, GLColor::red);
    std::vector<GLColor> green(4, GLColor::green);

    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, red.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    glViewport(0, 0, 2, 2);
    ASSERT_GL_NO_ERROR();

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw once to flush dirty state bits.
    draw2DTexturedQuad(0.5f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    // Update the (0, 1) pixel to be blue
    updateFunc(GL_TEXTURE_2D, &renderTarget, 0, 1, GLColor::blue);

    // Draw green to the right half of the Framebuffer.
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, green.data());
    glViewport(1, 0, 1, 2);
    draw2DTexturedQuad(0.5f, 1.0f, true);

    // Update the (1, 1) pixel to be yellow
    updateFunc(GL_TEXTURE_2D, &renderTarget, 1, 1, GLColor::yellow);

    ASSERT_GL_NO_ERROR();

    // Verify we have a quad with the right colors in the FBO.
    std::vector<GLColor> expected = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    std::vector<GLColor> actual(4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, actual.data());
    EXPECT_EQ(expected, actual);
}

// Tests that TexSubImage updates are flushed before rendering.
TEST_P(SimpleStateChangeTest, TexSubImageOnTextureBoundToFrambuffer)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    auto updateFunc = [](GLenum textureBinding, GLTexture *tex, GLint x, GLint y,
                         const GLColor &color) {
        glBindTexture(textureBinding, *tex);
        glTexSubImage2D(textureBinding, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
    };

    updateTextureBoundToFramebufferHelper(updateFunc);
}

// Tests that CopyTexSubImage updates are flushed before rendering.
TEST_P(SimpleStateChangeTest, CopyTexSubImageOnTextureBoundToFrambuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    GLTexture copySource;
    glBindTexture(GL_TEXTURE_2D, copySource);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer copyFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, copyFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copySource, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    auto updateFunc = [&copySource](GLenum textureBinding, GLTexture *tex, GLint x, GLint y,
                                    const GLColor &color) {
        glBindTexture(GL_TEXTURE_2D, copySource);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color.data());

        glBindTexture(textureBinding, *tex);
        glCopyTexSubImage2D(textureBinding, 0, x, y, 0, 0, 1, 1);
    };

    updateTextureBoundToFramebufferHelper(updateFunc);
}

// Tests that the read framebuffer doesn't affect what the draw call thinks the attachments are
// (which is what the draw framebuffer dictates) when a command is issued with the GL_FRAMEBUFFER
// target.
TEST_P(SimpleStateChangeTestES3, ReadFramebufferDrawFramebufferDifferentAttachments)
{
    // http://anglebug.com/4092
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    GLRenderbuffer drawColorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, drawColorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLRenderbuffer drawDepthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, drawDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1, 1);

    GLRenderbuffer readColorBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, readColorBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              drawColorBuffer);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              drawDepthBuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    GLFramebuffer readFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              readColorBuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // A handful of non-draw calls can sync framebuffer state, such as discard, invalidate,
    // invalidateSub and multisamplefv.
    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests that invalidate then copy then blend works.
TEST_P(SimpleStateChangeTestES3, InvalidateThenCopyThenBlend)
{
    // Create a framebuffer as the source of copy
    const GLColor kSrcData = GLColor::cyan;
    GLTexture copySrc;
    glBindTexture(GL_TEXTURE_2D, copySrc);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kSrcData);

    GLFramebuffer readFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copySrc, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    // Create the framebuffer that will be invalidated
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Copy into the framebuffer's texture.  The framebuffer should now be cyan.
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then blit then blend works.
TEST_P(SimpleStateChangeTestES3, InvalidateThenBlitThenBlend)
{
    // Create a framebuffer as the source of blit
    const GLColor kSrcData = GLColor::cyan;
    GLTexture blitSrc;
    glBindTexture(GL_TEXTURE_2D, blitSrc);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kSrcData);

    GLFramebuffer readFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blitSrc, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    // Create the framebuffer that will be invalidated
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Blit into the framebuffer.  The framebuffer should now be cyan.
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then generate mipmaps works
TEST_P(SimpleStateChangeTestES3, InvalidateThenGenerateMipmapsThenBlend)
{
    // Create a texture on which generate mipmaps would be called
    const GLColor kMip0Data[4] = {GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan};
    const GLColor kMip1Data    = GLColor::blue;
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kMip0Data);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kMip1Data);

    // Create the framebuffer that will be invalidated
    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then upload works
TEST_P(SimpleStateChangeTestES3, InvalidateThenUploadThenBlend)
{
    // http://anglebug.com/4870
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // Create the framebuffer that will be invalidated
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Upload data to it
    const GLColor kUploadColor = GLColor::cyan;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kUploadColor);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then sub upload works
TEST_P(SimpleStateChangeTestES3, InvalidateThenSubUploadThenBlend)
{
    // http://anglebug.com/4870
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // Create the framebuffer that will be invalidated
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Upload data to it
    const GLColor kUploadColor = GLColor::cyan;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &kUploadColor);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then compute write works
TEST_P(SimpleStateChangeTestES31, InvalidateThenStorageWriteThenBlend)
{
    // Fails on AMD OpenGL Windows. This configuration isn't maintained.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    // http://anglebug.com/5387
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
layout (rgba8, binding = 1) writeonly uniform highp image2D dstImage;
void main()
{
    imageStore(dstImage, ivec2(gl_GlobalInvocationID.xy), vec4(0.0f, 1.0f, 1.0f, 1.0f));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    // Create the framebuffer texture
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    glBindImageTexture(1, renderTarget, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Write to the texture with compute once.  In the Vulkan backend, this will make sure the image
    // is already created with STORAGE usage and avoids recreate later.
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Create the framebuffer that will be invalidated
    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Write to it with a compute shader
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that invalidate then compute write works inside PPO
TEST_P(SimpleStateChangeTestES31, InvalidateThenStorageWriteThenBlendPpo)
{
    // Fails on AMD OpenGL Windows. This configuration isn't maintained.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    // PPOs are only supported in the Vulkan backend
    ANGLE_SKIP_TEST_IF(!isVulkanRenderer());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
layout (rgba8, binding = 1) writeonly uniform highp image2D dstImage;
void main()
{
    imageStore(dstImage, ivec2(gl_GlobalInvocationID.xy), vec4(0.0f, 1.0f, 1.0f, 1.0f));
})";

    GLProgramPipeline pipeline;
    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glUseProgramStages(pipeline, GL_COMPUTE_SHADER_BIT, program);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgram(0);

    // Create the framebuffer texture
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    glBindImageTexture(1, renderTarget, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Write to the texture with compute once.  In the Vulkan backend, this will make sure the image
    // is already created with STORAGE usage and avoids recreate later.
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Create the framebuffer that will be invalidated
    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer and invalidate it.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment);
    EXPECT_GL_NO_ERROR();

    // Write to it with a compute shader
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(1.0f, 0.0f, 0.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that sub-invalidate then draw works.
TEST_P(SimpleStateChangeTestES3, SubInvalidateThenDraw)
{
    // Fails on AMD OpenGL Windows. This configuration isn't maintained.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());

    // Create the framebuffer that will be invalidated
    GLTexture renderTarget;
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer.
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw into a quarter of the framebuffer, then invalidate that same region.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glEnable(GL_SCISSOR_TEST);
    glScissor(1, 1, 1, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Only invalidate a quarter of the framebuffer.
    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateSubFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &invalidateAttachment, 1, 1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glDisable(GL_SCISSOR_TEST);

    // Blend into the framebuffer, then verify that the framebuffer should have had cyan.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    blendAndVerifyColor(GLColor32F(0.0f, 0.0f, 1.0f, 0.5f), GLColor(127, 127, 127, 191));
}

// Tests that mid-render-pass invalidate then clear works for color buffers.  This test ensures that
// the invalidate is undone on draw.
TEST_P(SimpleStateChangeTestES3, ColorInvalidateThenClear)
{
    // Create the framebuffer that will be invalidated
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Initialize the framebuffer with a draw call.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Invalidate it.
    GLenum invalidateAttachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalidateAttachment);

    // Clear the framebuffer.
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Expect the clear color, ensuring that invalidate wasn't applied after clear.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that mid-render-pass invalidate then clear works for depth buffers.  This test ensures that
// the invalidate is undone on draw.
TEST_P(SimpleStateChangeTestES3, DepthInvalidateThenClear)
{
    // Create the framebuffer that will be invalidated
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Initialize the framebuffer with a draw call.
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);

    // Invalidate depth.
    GLenum invalidateAttachment = GL_DEPTH_ATTACHMENT;
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalidateAttachment);

    // Clear the framebuffer.
    glClearDepthf(0.8f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Expect the draw color.  This breaks the render pass.  Later, the test ensures that invalidate
    // of depth wasn't applied after clear.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Blend with depth test and make sure depth is as expected.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glDepthFunc(GL_LESS);
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.59f);

    glDepthFunc(GL_GREATER);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.61f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Tests deleting a Framebuffer that is in use.
TEST_P(SimpleStateChangeTest, DeleteFramebufferInUse)
{
    constexpr int kSize = 16;

    // Create a simple framebuffer.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glViewport(0, 0, kSize, kSize);

    // Draw a solid red color to the framebuffer.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Delete the framebuffer while the call is in flight.
    framebuffer.reset();

    // Make a new framebuffer so we can read back the texture.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Flush via ReadPixels and check red was drawn.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// This test was made to reproduce a specific issue with our Vulkan backend where were releasing
// buffers too early. The test has 2 textures, we first create a texture and update it with
// multiple updates, but we don't use it right away, we instead draw using another texture
// then we bind the first texture and draw with it.
TEST_P(SimpleStateChangeTest, DynamicAllocationOfMemoryForTextures)
{
    constexpr int kSize = 64;

    GLuint program = get2DTexturedQuadProgram();
    glUseProgram(program);

    std::vector<GLColor> greenPixels(kSize * kSize, GLColor::green);
    std::vector<GLColor> redPixels(kSize * kSize, GLColor::red);
    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    for (int i = 0; i < 100; i++)
    {
        // We do this a lot of time to make sure we use multiple buffers in the vulkan backend.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                        greenPixels.data());
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ASSERT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, redPixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup the vertex array to draw a quad.
    GLint positionLocation = glGetAttribLocation(program, "position");
    setupQuadVertexBuffer(1.0f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Draw quad with texture 2 while texture 1 has "staged" changes that have not been flushed yet.
    glBindTexture(GL_TEXTURE_2D, texture2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // If we now try to draw with texture1, we should trigger the issue.
    glBindTexture(GL_TEXTURE_2D, texture1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests deleting a Framebuffer that is in use.
TEST_P(SimpleStateChangeTest, RedefineFramebufferInUse)
{
    constexpr int kSize = 16;

    // Create a simple framebuffer.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glViewport(0, 0, kSize, kSize);

    // Draw red to the framebuffer.
    simpleDrawWithColor(GLColor::red);

    // Change the framebuffer while the call is in flight to a new texture.
    GLTexture otherTexture;
    glBindTexture(GL_TEXTURE_2D, otherTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, otherTexture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Draw green to the framebuffer. Verify the color.
    simpleDrawWithColor(GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make a new framebuffer so we can read back the first texture and verify red.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Tests that redefining a Framebuffer Texture Attachment works as expected.
TEST_P(SimpleStateChangeTest, RedefineFramebufferTexture)
{
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Bind a simple 8x8 texture to the framebuffer, draw red.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glViewport(0, 0, 8, 8);
    simpleDrawWithColor(GLColor::red);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "first draw should be red";

    // Redefine the texture to 32x32, draw green. Verify we get what we expect.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glViewport(0, 0, 32, 32);
    simpleDrawWithColor(GLColor::green);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green) << "second draw should be green";
}

// Trips a bug in the Vulkan back-end where a Texture wouldn't transition correctly.
TEST_P(SimpleStateChangeTest, DrawAndClearTextureRepeatedly)
{
    // Fails on 431.02 driver. http://anglebug.com/3748
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsVulkan());

    // Fails on AMD OpenGL Windows. This configuration isn't maintained.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, uniLoc);
    glUniform1i(uniLoc, 0);

    const int numRowsCols = 2;
    const int cellSize    = getWindowWidth() / 2;

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed            = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, 0.0f, 1.0f);

            // Set the texture to a constant color using glClear and a user FBO.
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClearColor(color[0], color[1], color[2], color[3]);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw a small colored quad to the default FBO using the viewport.
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(cellX * cellSize, cellY * cellSize, cellSize, cellSize);
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        }
    }

    // Verify the colored quads were drawn correctly despite no flushing.
    std::vector<GLColor> pixelData(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());

    ASSERT_GL_NO_ERROR();

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed            = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, 0.0f, 1.0f);

            GLColor expectedColor(color);

            int testN =
                cellX * cellSize + cellY * getWindowWidth() * cellSize + getWindowWidth() + 1;
            GLColor actualColor = pixelData[testN];
            EXPECT_COLOR_NEAR(expectedColor, actualColor, 1);
        }
    }
}

// Test that clear followed by rebind of framebuffer attachment works (with noop clear in between).
TEST_P(SimpleStateChangeTestES3, ClearThenNoopClearThenRebindAttachment)
{
    // Create a texture with red
    const GLColor kInitColor1 = GLColor::red;
    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kInitColor1);

    // Create a framebuffer to be cleared
    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear again, but in a way that would be a no-op.  In the Vulkan backend, this will result in
    // a framebuffer sync state, which extracts deferred clears.  However, as the clear is actually
    // a noop, the deferred clears will remain unflushed.
    glClear(0);

    // Change framebuffer's attachment to the other texture.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // A bogus draw to make sure the render pass is cleared in the Vulkan backend.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Expect red, which is the original contents of texture1.  If the clear is mistakenly applied
    // to the new attachment, green will be read back.
    EXPECT_PIXEL_COLOR_EQ(0, 0, kInitColor1);

    // Attach back to texture2.  It should be cleared to green.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that clear followed by rebind of framebuffer attachment works (with 0-sized scissor clear in
// between).
TEST_P(SimpleStateChangeTestES3, ClearThenZeroSizeScissoredClearThenRebindAttachment)
{
    // Create a texture with red
    const GLColor kInitColor1 = GLColor::red;
    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kInitColor1);

    // Create a framebuffer to be cleared
    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear again, but in a way that would be a no-op.  In the Vulkan backend, this will result in
    // a framebuffer sync state, which extracts deferred clears.  However, as the clear is actually
    // a noop, the deferred clears will remain unflushed.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    // Change framebuffer's attachment to the other texture.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // A bogus draw to make sure the render pass is cleared in the Vulkan backend.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Expect red, which is the original contents of texture1.  If the clear is mistakenly applied
    // to the new attachment, green will be read back.
    EXPECT_PIXEL_COLOR_EQ(0, 0, kInitColor1);

    // Attach back to texture2.  It should be cleared to green.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that clear followed by rebind of framebuffer attachment works (with noop blit in between).
TEST_P(SimpleStateChangeTestES3, ClearThenNoopBlitThenRebindAttachment)
{
    // Create a texture with red
    const GLColor kInitColor1 = GLColor::red;
    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kInitColor1);

    // Create a framebuffer to be cleared
    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Clear the framebuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Issue noop blit.  In the Vulkan backend, this will result in a framebuffer sync state, which
    // extracts deferred clears.  However, as the blit is actually a noop, the deferred clears will
    // remain unflushed.
    GLTexture blitSrc;
    glBindTexture(GL_TEXTURE_2D, blitSrc);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer readFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blitSrc, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Change framebuffer's attachment to the other texture.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // A bogus draw to make sure the render pass is cleared in the Vulkan backend.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(program);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Expect red, which is the original contents of texture1.  If the clear is mistakenly applied
    // to the new attachment, green will be read back.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kInitColor1);

    // Attach back to texture2.  It should be cleared to green.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validates disabling cull face really disables it.
TEST_P(SimpleStateChangeTest, EnableAndDisableCullFace)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_FRONT);

    drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    // Disable cull face and redraw, then make sure we have the quad drawn.
    glDisable(GL_CULL_FACE);

    drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

TEST_P(SimpleStateChangeTest, ScissorTest)
{
    // This test validates this order of state changes:
    // 1- Set scissor but don't enable it, validate its not used.
    // 2- Enable it and validate its working.
    // 3- Disable the scissor validate its not used anymore.

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glClear(GL_COLOR_BUFFER_BIT);

    // Set the scissor region, but don't enable it yet.
    glScissor(getWindowWidth() / 4, getWindowHeight() / 4, getWindowWidth() / 2,
              getWindowHeight() / 2);

    // Fill the whole screen with a quad.
    drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    // Test outside, scissor isnt enabled so its red.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Test inside, red of the fragment shader.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    // Clear everything and start over with the test enabled.
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    // Test outside the scissor test, pitch black.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    // Test inside, red of the fragment shader.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    // Now disable the scissor test, do it again, and verify the region isn't used
    // for the scissor test.
    glDisable(GL_SCISSOR_TEST);

    // Clear everything and start over with the test enabled.
    glClear(GL_COLOR_BUFFER_BIT);

    drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);

    ASSERT_GL_NO_ERROR();

    // Test outside, scissor isnt enabled so its red.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Test inside, red of the fragment shader.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);
}

// This test validates we are able to change the valid of a uniform dynamically.
TEST_P(SimpleStateChangeTest, UniformUpdateTest)
{
    constexpr char kPositionUniformVertexShader[] = R"(
precision mediump float;
attribute vec2 position;
uniform vec2 uniPosModifier;
void main()
{
    gl_Position = vec4(position + uniPosModifier, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kPositionUniformVertexShader, essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint posUniformLocation = glGetUniformLocation(program, "uniPosModifier");
    ASSERT_NE(posUniformLocation, -1);
    GLint colorUniformLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // draw a red quad to the left side.
    glUniform2f(posUniformLocation, -0.5, 0.0);
    glUniform4f(colorUniformLocation, 1.0, 0.0, 0.0, 1.0);
    drawQuad(program.get(), "position", 0.0f, 0.5f, true);

    // draw a green quad to the right side.
    glUniform2f(posUniformLocation, 0.5, 0.0);
    glUniform4f(colorUniformLocation, 0.0, 1.0, 0.0, 1.0);
    drawQuad(program.get(), "position", 0.0f, 0.5f, true);

    ASSERT_GL_NO_ERROR();

    // Test the center of the left quad. Should be red.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 2, GLColor::red);

    // Test the center of the right quad. Should be green.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4 * 3, getWindowHeight() / 2, GLColor::green);
}

// Tests that changing the storage of a Renderbuffer currently in use by GL works as expected.
TEST_P(SimpleStateChangeTest, RedefineRenderbufferInUse)
{
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ANGLE_GL_PROGRAM(program, kSimpleVertexShader, kSimpleFragmentShader);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    // Set up and draw red to the left half the screen.
    std::vector<GLColor> redData(6, GLColor::red);
    GLBuffer vertexBufferRed;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferRed);
    glBufferData(GL_ARRAY_BUFFER, redData.size() * sizeof(GLColor), redData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    glViewport(0, 0, 16, 16);
    drawQuad(program, "position", 0.5f, 1.0f, true);

    // Immediately redefine the Renderbuffer.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);

    // Set up and draw green to the right half of the screen.
    std::vector<GLColor> greenData(6, GLColor::green);
    GLBuffer vertexBufferGreen;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferGreen);
    glBufferData(GL_ARRAY_BUFFER, greenData.size() * sizeof(GLColor), greenData.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    glViewport(0, 0, 64, 64);
    drawQuad(program, "position", 0.5f, 1.0f, true);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validate that we can draw -> change frame buffer size -> draw and we'll be rendering
// at the full size of the new framebuffer.
TEST_P(SimpleStateChangeTest, ChangeFramebufferSizeBetweenTwoDraws)
{
    constexpr size_t kSmallTextureSize = 2;
    constexpr size_t kBigTextureSize   = 4;

    // Create 2 textures, one of 2x2 and the other 4x4
    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSmallTextureSize, kSmallTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kBigTextureSize, kBigTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // A framebuffer for each texture to draw on.
    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint uniformLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(uniformLocation, -1);

    // Bind to the first framebuffer for drawing.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);

    // Set a scissor, that will trigger setting the internal scissor state in Vulkan to
    // (0,0,framebuffer.width, framebuffer.height) size since the scissor isn't enabled.
    glScissor(0, 0, 16, 16);
    ASSERT_GL_NO_ERROR();

    // Set color to red.
    glUniform4f(uniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, kSmallTextureSize, kSmallTextureSize);

    // Draw a full sized red quad
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Bind to the second (bigger) framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glViewport(0, 0, kBigTextureSize, kBigTextureSize);

    ASSERT_GL_NO_ERROR();

    // Set color to green.
    glUniform4f(uniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);

    // Draw again and we should fill everything with green and expect everything to be green.
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, kBigTextureSize, kBigTextureSize, GLColor::green);
}

// Tries to relink a program in use and use it again to draw something else.
TEST_P(SimpleStateChangeTest, RelinkProgram)
{
    // http://anglebug.com/4121
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGLES());
    const GLuint program = glCreateProgram();

    GLuint vs     = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    GLuint blueFs = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Blue());
    GLuint redFs  = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Red());

    glAttachShader(program, vs);
    glAttachShader(program, blueFs);

    glLinkProgram(program);
    CheckLinkStatusAndReturnProgram(program, true);

    glClear(GL_COLOR_BUFFER_BIT);
    std::vector<Vector3> vertices = {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f},
                                     {-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0, 1.0f, 0.0f}};
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Draw a blue triangle to the right
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Relink to draw red to the left
    glDetachShader(program, blueFs);
    glAttachShader(program, redFs);

    glLinkProgram(program);
    CheckLinkStatusAndReturnProgram(program, true);

    glDrawArrays(GL_TRIANGLES, 3, 3);

    ASSERT_GL_NO_ERROR();

    glDisableVertexAttribArray(positionLocation);

    // Verify we drew red and green in the right places.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 2, GLColor::red);

    glDeleteShader(vs);
    glDeleteShader(blueFs);
    glDeleteShader(redFs);
    glDeleteProgram(program);
}

// Creates a program that uses uniforms and then immediately release it and then use it. Should be
// valid.
TEST_P(SimpleStateChangeTest, ReleaseShaderInUseThatReadsFromUniforms)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    const GLint uniformLoc = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    EXPECT_NE(-1, uniformLoc);

    // Set color to red.
    glUniform4f(uniformLoc, 1.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    std::vector<Vector3> vertices = {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Release program while its in use.
    glDeleteProgram(program);

    // Draw a red triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Set color to green
    glUniform4f(uniformLoc, 1.0f, 0.0f, 0.0f, 1.0f);

    // Draw a green triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ASSERT_GL_NO_ERROR();

    glDisableVertexAttribArray(positionLocation);

    glUseProgram(0);

    // Verify we drew red in the end since thats the last draw.
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, 0, GLColor::red);
}

// Tests that sampler sync isn't masked by program textures.
TEST_P(SimpleStateChangeTestES3, SamplerSyncNotTiedToProgram)
{
    // Create a sampler with NEAREST filtering.
    GLSampler sampler;
    glBindSampler(0, sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw with a program that uses no textures.
    ANGLE_GL_PROGRAM(program1, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(program1, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Create a simple texture with four colors and linear filtering.
    constexpr GLsizei kSize       = 2;
    std::array<GLColor, 4> pixels = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    GLTexture redTex;
    glBindTexture(GL_TEXTURE_2D, redTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create a program that uses the texture.
    constexpr char kVS[] = R"(attribute vec4 position;
varying vec2 texCoord;
void main()
{
    gl_Position = position;
    texCoord = position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 texCoord;
uniform sampler2D tex;
void main()
{
    gl_FragColor = texture2D(tex, texCoord);
})";

    // Draw. The sampler should override the clamp wrap mode with nearest.
    ANGLE_GL_PROGRAM(program2, kVS, kFS);
    ASSERT_EQ(0, glGetUniformLocation(program2, "tex"));
    drawQuad(program2, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    constexpr int kHalfSize = kWindowSize / 2;

    EXPECT_PIXEL_RECT_EQ(0, 0, kHalfSize, kHalfSize, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kHalfSize, 0, kHalfSize, kHalfSize, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, kHalfSize, kHalfSize, kHalfSize, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(kHalfSize, kHalfSize, kHalfSize, kHalfSize, GLColor::yellow);
}

// Tests different samplers can be used with same texture obj on different tex units.
TEST_P(SimpleStateChangeTestES3, MultipleSamplersWithSingleTextureObject)
{
    // Test overview - Create two separate sampler objects, initially with the same
    // sampling args (NEAREST). Bind the same texture object to separate texture units.
    // FS samples from two samplers and blends result.
    // Bind separate sampler objects to the same texture units as the texture object.
    // Render & verify initial results
    // Next modify sampler0 to have LINEAR filtering instead of NEAREST
    // Render and save results
    // Now restore sampler0 to NEAREST filtering and make sampler1 LINEAR
    // Render and verify results are the same as previous

    // Create 2 samplers with NEAREST filtering.
    constexpr GLsizei kNumSamplers = 2;
    // We create/bind an extra sampler w/o bound tex object for testing purposes
    GLSampler samplers[kNumSamplers + 1];
    // Set samplers to initially have same state w/ NEAREST filter mode
    for (uint32_t i = 0; i < kNumSamplers + 1; ++i)
    {
        glSamplerParameteri(samplers[i], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(samplers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(samplers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(samplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(samplers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameterf(samplers[i], GL_TEXTURE_MAX_LOD, 1000);
        glSamplerParameterf(samplers[i], GL_TEXTURE_MIN_LOD, -1000);
        glBindSampler(i, samplers[i]);
        ASSERT_GL_NO_ERROR();
    }

    // Create a simple texture with four colors
    constexpr GLsizei kSize       = 2;
    std::array<GLColor, 4> pixels = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    GLTexture rgbyTex;
    // Bind same texture object to tex units 0 & 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbyTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rgbyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());

    // Create a program that uses the texture with 2 separate samplers.
    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D samp1;
uniform sampler2D samp2;
void main()
{
    gl_FragColor = mix(texture2D(samp1, v_texCoord), texture2D(samp2, v_texCoord), 0.5);
})";

    // Create program and bind samplers to tex units 0 & 1
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    GLint s1loc = glGetUniformLocation(program, "samp1");
    GLint s2loc = glGetUniformLocation(program, "samp2");
    glUseProgram(program);
    glUniform1i(s1loc, 0);
    glUniform1i(s2loc, 1);
    // Draw. This first draw is a confidence check and not really necessary for the test
    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.5f);
    ASSERT_GL_NO_ERROR();

    constexpr int kHalfSize = kWindowSize / 2;

    // When rendering w/ NEAREST, colors are all maxed out so should still be solid
    EXPECT_PIXEL_RECT_EQ(0, 0, kHalfSize, kHalfSize, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kHalfSize, 0, kHalfSize, kHalfSize, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, kHalfSize, kHalfSize, kHalfSize, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(kHalfSize, kHalfSize, kHalfSize, kHalfSize, GLColor::yellow);

    // Make first sampler use linear filtering
    glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.5f);
    ASSERT_GL_NO_ERROR();
    // Capture rendered pixel color with s0 linear
    std::vector<GLColor> s0LinearColors(kWindowSize * kWindowSize);
    glReadPixels(0, 0, kWindowSize, kWindowSize, GL_RGBA, GL_UNSIGNED_BYTE, s0LinearColors.data());

    // Now restore first sampler & update second sampler
    glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.5f);
    ASSERT_GL_NO_ERROR();
    // Capture rendered pixel color w/ s1 linear
    std::vector<GLColor> s1LinearColors(kWindowSize * kWindowSize);
    glReadPixels(0, 0, kWindowSize, kWindowSize, GL_RGBA, GL_UNSIGNED_BYTE, s1LinearColors.data());
    // Results should be the same regardless of if s0 or s1 is linear
    EXPECT_EQ(s0LinearColors, s1LinearColors);
}

// Tests that rendering works as expected with multiple VAOs.
TEST_P(SimpleStateChangeTestES31, MultipleVertexArrayObjectRendering)
{
    constexpr char kVertexShader[] = R"(attribute vec4 a_position;
attribute vec4 a_color;
varying vec4 v_color;
void main()
{
    gl_Position = a_position;
    v_color = a_color;
})";

    constexpr char kFragmentShader[] = R"(precision mediump float;
varying vec4 v_color;
void main()
{
    gl_FragColor = v_color;
})";

    ANGLE_GL_PROGRAM(mProgram, kVertexShader, kFragmentShader);
    GLint positionLoc = glGetAttribLocation(mProgram, "a_position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(mProgram, "a_color");
    ASSERT_NE(-1, colorLoc);

    GLVertexArray VAOS[2];
    GLBuffer positionBuffer;
    GLBuffer colorBuffer;
    const auto quadVertices = GetQuadVertices();

    glBindVertexArray(VAOS[0]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_BYTE, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    std::vector<GLColor32F> blueColor(6, kFloatBlue);
    glBufferData(GL_ARRAY_BUFFER, blueColor.size() * sizeof(GLColor32F), blueColor.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_BYTE, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(VAOS[1]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    std::vector<GLColor32F> greenColor(6, kFloatGreen);
    glBufferData(GL_ARRAY_BUFFER, greenColor.size() * sizeof(GLColor32F), greenColor.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(mProgram);
    ASSERT_GL_NO_ERROR();

    glBindVertexArray(VAOS[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // This drawing should not affect the next drawing.
    glBindVertexArray(VAOS[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(VAOS[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 2, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Tests that consecutive identical draw calls that write to an image in the fragment shader work
// correctly.  This requires a memory barrier in between the draw calls which should not be
// reordered w.r.t the calls.
TEST_P(SimpleStateChangeTestES31, DrawWithImageTextureThenDrawAgain)
{
    // http://anglebug.com/5593
    ANGLE_SKIP_TEST_IF(IsD3D11());

    GLint maxFragmentImageUniforms;
    glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &maxFragmentImageUniforms);

    ANGLE_SKIP_TEST_IF(maxFragmentImageUniforms < 1);

    // The test uses a GL_R32F framebuffer.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_float"));

    constexpr GLsizei kSize = 1;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    const float kInitialValue = 0.125f;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, kSize, kSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RED, GL_FLOAT, &kInitialValue);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer readbackFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readbackFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    // Create a program that outputs to the image in the fragment shader.
    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(r32f, binding = 0) uniform highp image2D dst;
out vec4 colorOut;
void main()
{
    vec4 result = imageLoad(dst, ivec2(gl_FragCoord.xy));
    colorOut = result;

    result.x += 0.193;
    imageStore(dst, ivec2(gl_FragCoord.xy), result);
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint positionLoc = glGetAttribLocation(program, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    EXPECT_GL_NO_ERROR();

    // Setup the draw so that there's no state change between the draw calls.
    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the output of the two draw calls through the image is correct
    EXPECT_PIXEL_COLOR32F_NEAR(0, 0, GLColor32F(kInitialValue + 0.193f * 2, 0.0f, 0.0f, 1.0f),
                               0.001f);

    // Verify the output of rendering as well
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(81, 0, 0, 255), 1);
}

// Tests that sampling from a texture in one draw call followed by writing to its image in another
// draw call works correctly.  This requires a barrier in between the draw calls.
TEST_P(SimpleStateChangeTestES31, DrawWithTextureThenDrawWithImage)
{
    // http://anglebug.com/5593
    ANGLE_SKIP_TEST_IF(IsD3D11());
    // http://anglebug.com/5686
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsDesktopOpenGL());

    GLint maxFragmentImageUniforms;
    glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &maxFragmentImageUniforms);

    ANGLE_SKIP_TEST_IF(maxFragmentImageUniforms < 1);

    constexpr GLsizei kSize = 1;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_NO_ERROR();

    const GLColor kInitialColor = GLColor::red;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    &kInitialColor);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer readbackFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readbackFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // Create a program that samples from the texture in the fragment shader.
    constexpr char kFSSample[] = R"(#version 310 es
precision mediump float;
uniform sampler2D tex;
out vec4 colorOut;
void main()
{
    colorOut = texture(tex, vec2(0));
})";
    ANGLE_GL_PROGRAM(sampleProgram, essl31_shaders::vs::Simple(), kFSSample);

    // Create a program that outputs to the image in the fragment shader.
    constexpr char kFSWrite[] = R"(#version 310 es
precision mediump float;
layout(rgba8, binding = 0) uniform highp writeonly image2D dst;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 0, 1.0, 0);
    imageStore(dst, ivec2(gl_FragCoord.xy), vec4(0.0, 1.0, 0.0, 1.0));
})";

    ANGLE_GL_PROGRAM(writeProgram, essl31_shaders::vs::Simple(), kFSWrite);

    glUseProgram(sampleProgram);
    GLint positionLocSample = glGetAttribLocation(sampleProgram, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocSample);

    glUseProgram(writeProgram);
    GLint positionLocWrite = glGetAttribLocation(writeProgram, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocWrite);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

    // Setup the draw so that there's no state change between the draw calls.
    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocSample, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(positionLocWrite, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocSample);
    glEnableVertexAttribArray(positionLocWrite);

    glUseProgram(sampleProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glUseProgram(writeProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the output of the two draw calls through the image is correct
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the output of rendering as well
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
}

// Tests that reading from a ubo in one draw call followed by writing to it as SSBO in another draw
// call works correctly.  This requires a barrier in between the draw calls.
TEST_P(SimpleStateChangeTestES31, DrawWithUBOThenDrawWithSSBO)
{
    // http://anglebug.com/5593
    ANGLE_SKIP_TEST_IF(IsD3D11());

    GLint maxFragmentShaderStorageBlocks;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);

    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks < 1);

    constexpr GLsizei kSize = 1;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_NO_ERROR();

    constexpr std::array<float, 4> kBufferInitValue = {0.125f, 0.25f, 0.5f, 1.0f};
    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kBufferInitValue), kBufferInitValue.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Create a program that reads from the ubo in the fragment shader.
    constexpr char kFSRead[] = R"(#version 310 es
precision mediump float;
uniform block { vec4 vec; } b;
out vec4 colorOut;
void main()
{
    colorOut = b.vec;
})";
    ANGLE_GL_PROGRAM(readProgram, essl31_shaders::vs::Simple(), kFSRead);

    // Create a program that outputs to the image in the fragment shader.
    constexpr char kFSWrite[] = R"(#version 310 es
precision mediump float;
layout(binding = 0, std430) buffer Output {
    vec4 vec;
} b;
out vec4 colorOut;
void main()
{
    b.vec = vec4(0.7, 0.6, 0.4, 0.3);
    colorOut = vec4(0.125, 0.125, 0.125, 0);
})";

    ANGLE_GL_PROGRAM(writeProgram, essl31_shaders::vs::Simple(), kFSWrite);

    glUseProgram(readProgram);
    GLint positionLocRead = glGetAttribLocation(readProgram, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocRead);

    glUseProgram(writeProgram);
    GLint positionLocWrite = glGetAttribLocation(writeProgram, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocWrite);

    // Setup the draw so that there's no state change between the draw calls.
    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocRead, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(positionLocWrite, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocRead);
    glEnableVertexAttribArray(positionLocWrite);

    glUseProgram(readProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(writeProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the output of rendering
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(63, 95, 159, 255), 1);

    // Verify the output from the second draw call
    const float *ptr = reinterpret_cast<const float *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferInitValue), GL_MAP_READ_BIT));

    EXPECT_NEAR(ptr[0], 0.7f, 0.001);
    EXPECT_NEAR(ptr[1], 0.6f, 0.001);
    EXPECT_NEAR(ptr[2], 0.4f, 0.001);
    EXPECT_NEAR(ptr[3], 0.3f, 0.001);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Tests that rendering to a texture in one draw call followed by sampling from it in a dispatch
// call works correctly.  This requires an implicit barrier in between the calls.
TEST_P(SimpleStateChangeTestES31, DrawThenSampleWithCompute)
{
    // TODO(anglebug.com/5649): Test is failing since it was introduced on Linux AMD GLES
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD() && IsLinux());

    constexpr GLsizei kSize = 1;
    const GLColor kInitColor(111, 222, 33, 44);

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, &kInitColor);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    constexpr std::array<float, 4> kBufferInitValue = {0.123f, 0.456f, 0.789f, 0.852f};
    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferInitValue), kBufferInitValue.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(drawProgram, essl31_shaders::vs::Simple(), essl31_shaders::fs::Red());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
uniform sampler2D tex;
layout(binding = 0, std430) buffer Output {
    vec4 vec;
} b;
void main()
{
    b.vec = texelFetch(tex, ivec2(gl_LocalInvocationID.xy), 0);
})";

    ANGLE_GL_COMPUTE_PROGRAM(readProgram, kCS);
    glUseProgram(readProgram);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(readProgram, "tex"), 0);

    drawQuad(drawProgram, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    glUseProgram(readProgram);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the output of rendering
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Verify the output from the compute shader
    const float *ptr = reinterpret_cast<const float *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferInitValue), GL_MAP_READ_BIT));

    EXPECT_EQ(ptr[0], 1.0f);
    EXPECT_EQ(ptr[1], 0.0f);
    EXPECT_EQ(ptr[2], 0.0f);
    EXPECT_EQ(ptr[3], 1.0f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Tests that clearing a texture followed by sampling from it in a dispatch call works correctly.
// In the Vulkan backend, the clear is deferred and should be flushed correctly.
TEST_P(SimpleStateChangeTestES31, ClearThenSampleWithCompute)
{
    // http://anglebug.com/5687
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    constexpr GLsizei kSize = 1;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    // Make sure the update to the texture is effective.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Clear the texture through the framebuffer
    glClearColor(0, 1.0f, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr std::array<float, 4> kBufferInitValue = {0.123f, 0.456f, 0.789f, 0.852f};
    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferInitValue), kBufferInitValue.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
uniform sampler2D tex;
layout(binding = 0, std430) buffer Output {
    vec4 vec;
} b;
void main()
{
    b.vec = texelFetch(tex, ivec2(gl_LocalInvocationID.xy), 0);
})";

    ANGLE_GL_COMPUTE_PROGRAM(readProgram, kCS);
    glUseProgram(readProgram);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(readProgram, "tex"), 0);

    glUseProgram(readProgram);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the clear
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the output from the compute shader
    const float *ptr = reinterpret_cast<const float *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferInitValue), GL_MAP_READ_BIT));

    EXPECT_EQ(ptr[0], 0.0f);
    EXPECT_EQ(ptr[1], 1.0f);
    EXPECT_EQ(ptr[2], 0.0f);
    EXPECT_EQ(ptr[3], 1.0f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Tests that writing to a buffer with transform feedback in one draw call followed by reading from
// it in a dispatch call works correctly.  This requires an implicit barrier in between the calls.
TEST_P(SimpleStateChangeTestES31, TransformFeedbackThenReadWithCompute)
{
    // http://anglebug.com/5687
    ANGLE_SKIP_TEST_IF(IsAMD() && IsVulkan());

    constexpr GLsizei kBufferSize = sizeof(float) * 4 * 6;
    GLBuffer buffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);

    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, essl3_shaders::vs::Simple(),
                                        essl3_shaders::fs::Green(), tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);
    glUseProgram(program);

    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.0f);
    glEndTransformFeedback();

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
layout(binding = 0) uniform Input
{
    vec4 data[3];
};
layout(binding = 0, std430) buffer Output {
    bool pass;
};
void main()
{
    pass = data[0] == vec4(-1, 1, 0, 1) &&
           data[1] == vec4(-1, -1, 0, 1) &&
           data[2] == vec4(1, -1, 0, 1);
})";

    ANGLE_GL_COMPUTE_PROGRAM(readProgram, kCS);
    glUseProgram(readProgram);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);

    constexpr GLsizei kResultSize = sizeof(uint32_t);
    GLBuffer resultBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kResultSize, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, resultBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify the output of rendering
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the output from the compute shader
    const uint32_t *ptr = reinterpret_cast<const uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kResultSize, GL_MAP_READ_BIT));

    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Tests that deleting an in-flight image texture does not immediately delete the resource.
TEST_P(SimpleStateChangeTestComputeES31, DeleteImageTextureInUse)
{
    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    GLTexture texRead;
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    EXPECT_GL_NO_ERROR();

    glUseProgram(mProgram);

    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);
    texRead.reset();

    std::array<GLColor, 4> results;
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, results.data());
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(colors[i], results[i]);
    }
}

// Tests that bind the same image texture all the time between different dispatch calls.
TEST_P(SimpleStateChangeTestComputeES31, RebindImageTextureDispatchAgain)
{
    std::array<GLColor, 4> colors = {{GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan}};
    GLTexture texRead;
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());

    glUseProgram(mProgram);

    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    // Bind again
    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, 2, 2, GLColor::cyan);
}

// Tests that we can dispatch with an image texture, modify the image texture with a texSubImage,
// and then dispatch again correctly.
TEST_P(SimpleStateChangeTestComputeES31, DispatchWithImageTextureTexSubImageThenDispatchAgain)
{
    std::array<GLColor, 4> colors    = {{GLColor::red, GLColor::red, GLColor::red, GLColor::red}};
    std::array<GLColor, 4> subColors = {
        {GLColor::green, GLColor::green, GLColor::green, GLColor::green}};

    GLTexture texRead;
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());

    glUseProgram(mProgram);

    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    // Update bottom-half of image texture with green.
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, subColors.data());
    ASSERT_GL_NO_ERROR();

    // Dispatch again, should still work.
    glDispatchCompute(1, 1, 1);
    ASSERT_GL_NO_ERROR();

    // Validate first half of the image is red and the bottom is green.
    std::array<GLColor, 4> results;
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, results.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::green, results[0]);
    EXPECT_EQ(GLColor::green, results[1]);
    EXPECT_EQ(GLColor::red, results[2]);
    EXPECT_EQ(GLColor::red, results[3]);
}

// Test updating an image texture's contents while in use by GL works as expected.
TEST_P(SimpleStateChangeTestComputeES31, UpdateImageTextureInUse)
{
    std::array<GLColor, 4> rgby = {{GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture texRead;
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, rgby.data());

    glUseProgram(mProgram);

    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    // Update the texture to be YBGR, while the Texture is in-use. Should not affect the dispatch.
    std::array<GLColor, 4> ybgr = {{GLColor::yellow, GLColor::blue, GLColor::green, GLColor::red}};
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, ybgr.data());
    ASSERT_GL_NO_ERROR();

    // Check the Framebuffer. The dispatch call should have completed with the original RGBY data.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::yellow);

    // Dispatch again. The second dispatch call should use the updated YBGR data.
    glDispatchCompute(1, 1, 1);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test that we can alternate between image textures between different dispatchs.
TEST_P(SimpleStateChangeTestComputeES31, DispatchImageTextureAThenTextureBThenTextureA)
{
    std::array<GLColor, 4> colorsTexA = {
        {GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan}};

    std::array<GLColor, 4> colorsTexB = {
        {GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta}};

    GLTexture texA;
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colorsTexA.data());
    GLTexture texB;
    glBindTexture(GL_TEXTURE_2D, texB);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colorsTexB.data());

    glUseProgram(mProgram);

    glBindImageTexture(0, texA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(0, texB, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(0, texA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, 2, 2, GLColor::cyan);
    ASSERT_GL_NO_ERROR();
}

// Copied from SimpleStateChangeTestComputeES31::DeleteImageTextureInUse
// Tests that deleting an in-flight image texture does not immediately delete the resource.
TEST_P(SimpleStateChangeTestComputeES31PPO, DeleteImageTextureInUse)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    glGenFramebuffers(1, &mFramebuffer);
    glGenTextures(1, &mTexture);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    EXPECT_GL_NO_ERROR();

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=2, local_size_y=2) in;
layout (rgba8, binding = 0) readonly uniform highp image2D srcImage;
layout (rgba8, binding = 1) writeonly uniform highp image2D dstImage;
void main()
{
imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
           imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)));
})";

    bindProgramPipeline(kCS);

    glBindImageTexture(1, mTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

    ASSERT_GL_NO_ERROR();

    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    GLTexture texRead;
    glBindTexture(GL_TEXTURE_2D, texRead);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texRead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDispatchCompute(1, 1, 1);
    texRead.reset();

    std::array<GLColor, 4> results;
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, results.data());
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(colors[i], results[i]);
    }
}

static constexpr char kColorVS[] = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
})";

static constexpr char kColorFS[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
})";

class ValidationStateChangeTest : public ANGLETest
{
  protected:
    ValidationStateChangeTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class WebGL2ValidationStateChangeTest : public ValidationStateChangeTest
{
  protected:
    WebGL2ValidationStateChangeTest() { setWebGLCompatibilityEnabled(true); }
};

class ValidationStateChangeTestES31 : public ANGLETest
{};

class WebGLComputeValidationStateChangeTest : public ANGLETest
{
  public:
    WebGLComputeValidationStateChangeTest() { setWebGLCompatibilityEnabled(true); }
};

class RobustBufferAccessWebGL2ValidationStateChangeTest : public WebGL2ValidationStateChangeTest
{
  protected:
    RobustBufferAccessWebGL2ValidationStateChangeTest()
    {
        // SwS/OSX GL do not support robustness. Mali does not support it.
        if (!isSwiftshader() && !IsOSX() && !IsIOS() && !IsARM())
        {
            setRobustAccess(true);
        }
    }
};

// Tests that mapping and unmapping an array buffer in various ways causes rendering to fail.
// This isn't guaranteed to produce an error by GL. But we assume ANGLE always errors.
TEST_P(ValidationStateChangeTest, MapBufferAndDraw)
{
    // Initialize program and set up state.
    ANGLE_GL_PROGRAM(program, kColorVS, kColorFS);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);

    // Start with position enabled.
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    std::vector<GLColor> colorVertices(6, GLColor::blue);
    const size_t colorBufferSize = sizeof(GLColor) * 6;

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colorBufferSize, colorVertices.data(), GL_STATIC_DRAW);

    // Start with color disabled.
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDisableVertexAttribArray(colorLoc);

    ASSERT_GL_NO_ERROR();

    // Draw without a mapped buffer. Should succeed.
    glVertexAttrib4f(colorLoc, 0, 1, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Map position buffer and draw. Should fail.
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glMapBufferRange(GL_ARRAY_BUFFER, 0, posBufferSize, GL_MAP_READ_BIT);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Map position buffer and draw should fail.";
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // Map then enable color buffer. Should fail.
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glMapBufferRange(GL_ARRAY_BUFFER, 0, colorBufferSize, GL_MAP_READ_BIT);
    glEnableVertexAttribArray(colorLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Map then enable color buffer should fail.";

    // Unmap then draw. Should succeed.
    glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests that mapping an immutable and persistent buffer after calling glVertexAttribPointer()
// allows rendering to succeed.
TEST_P(ValidationStateChangeTest, MapImmutablePersistentBufferAndDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    // Initialize program and set up state.
    ANGLE_GL_PROGRAM(program, kColorVS, kColorFS);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(),
                       GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    // Start with position enabled.
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    std::vector<GLColor> colorVertices(6, GLColor::blue);
    const size_t colorBufferSize = sizeof(GLColor) * 6;

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, colorBufferSize, colorVertices.data(),
                       GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    // Start with color disabled.
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDisableVertexAttribArray(colorLoc);

    ASSERT_GL_NO_ERROR();

    // Draw without a mapped buffer. Should succeed.
    glVertexAttrib4f(colorLoc, 0, 1, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Map position buffer and draw. Should succeed.
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glMapBufferRange(GL_ARRAY_BUFFER, 0, posBufferSize,
                     GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // Map then enable color buffer. Should succeed.
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glMapBufferRange(GL_ARRAY_BUFFER, 0, colorBufferSize,
                     GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);
    glEnableVertexAttribArray(colorLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Unmap then draw. Should succeed.
    glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests that mapping an immutable and persistent buffer before calling glVertexAttribPointer()
// allows rendering to succeed. This case is special in that the VertexArray is not observing the
// buffer yet, so it's various cached buffer states aren't updated when the buffer is mapped.
TEST_P(ValidationStateChangeTest, MapImmutablePersistentBufferThenVAPAndDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    // Initialize program and set up state.
    ANGLE_GL_PROGRAM(program, kColorVS, kColorFS);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(),
                       GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    glMapBufferRange(GL_ARRAY_BUFFER, 0, posBufferSize,
                     GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    // Start with position enabled.
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    std::vector<GLColor> colorVertices(6, GLColor::blue);
    const size_t colorBufferSize = sizeof(GLColor) * 6;

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, colorBufferSize, colorVertices.data(),
                       GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    glMapBufferRange(GL_ARRAY_BUFFER, 0, colorBufferSize,
                     GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT);
    ASSERT_GL_NO_ERROR();

    // Start with color disabled.
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDisableVertexAttribArray(colorLoc);

    ASSERT_GL_NO_ERROR();

    // Draw without a mapped buffer. Should succeed.
    glVertexAttrib4f(colorLoc, 0, 1, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Unmap then draw. Should succeed.
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(colorLoc);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests that changing a vertex binding with glVertexAttribDivisor updates the mapped buffer check.
TEST_P(ValidationStateChangeTestES31, MapBufferAndDrawWithDivisor)
{
    // Seems to trigger a GL error in some edge cases. http://anglebug.com/2755
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsNVIDIA());

    // Initialize program and set up state.
    ANGLE_GL_PROGRAM(program, kColorVS, kColorFS);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    // Create a user vertex array.
    GLVertexArray vao;
    glBindVertexArray(vao);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);

    // Start with position enabled.
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    std::vector<GLColor> blueVertices(6, GLColor::blue);
    const size_t blueBufferSize = sizeof(GLColor) * 6;

    GLBuffer blueBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, blueBuffer);
    glBufferData(GL_ARRAY_BUFFER, blueBufferSize, blueVertices.data(), GL_STATIC_DRAW);

    // Start with color enabled at an unused binding.
    constexpr GLint kUnusedBinding = 3;
    ASSERT_NE(colorLoc, kUnusedBinding);
    ASSERT_NE(positionLoc, kUnusedBinding);
    glVertexAttribFormat(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glVertexAttribBinding(colorLoc, kUnusedBinding);
    glBindVertexBuffer(kUnusedBinding, blueBuffer, 0, sizeof(GLColor));
    glEnableVertexAttribArray(colorLoc);

    // Make binding 'colorLoc' use a mapped buffer.
    std::vector<GLColor> greenVertices(6, GLColor::green);
    const size_t greenBufferSize = sizeof(GLColor) * 6;
    GLBuffer greenBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, greenBuffer);
    glBufferData(GL_ARRAY_BUFFER, greenBufferSize, greenVertices.data(), GL_STATIC_DRAW);
    glMapBufferRange(GL_ARRAY_BUFFER, 0, greenBufferSize, GL_MAP_READ_BIT);
    glBindVertexBuffer(colorLoc, greenBuffer, 0, sizeof(GLColor));

    ASSERT_GL_NO_ERROR();

    // Draw without a mapped buffer. Should succeed.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Change divisor with VertexAttribDivisor. Should fail.
    glVertexAttribDivisor(colorLoc, 0);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "draw with mapped buffer should fail.";

    // Unmap the buffer. Should succeed.
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that changing a vertex binding with glVertexAttribDivisor updates the buffer size check.
TEST_P(WebGLComputeValidationStateChangeTest, DrawPastEndOfBufferWithDivisor)
{
    // Initialize program and set up state.
    ANGLE_GL_PROGRAM(program, kColorVS, kColorFS);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    // Create a user vertex array.
    GLVertexArray vao;
    glBindVertexArray(vao);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);

    // Start with position enabled.
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    std::vector<GLColor> blueVertices(6, GLColor::blue);
    const size_t blueBufferSize = sizeof(GLColor) * 6;

    GLBuffer blueBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, blueBuffer);
    glBufferData(GL_ARRAY_BUFFER, blueBufferSize, blueVertices.data(), GL_STATIC_DRAW);

    // Start with color enabled at an unused binding.
    constexpr GLint kUnusedBinding = 3;
    ASSERT_NE(colorLoc, kUnusedBinding);
    ASSERT_NE(positionLoc, kUnusedBinding);
    glVertexAttribFormat(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
    glVertexAttribBinding(colorLoc, kUnusedBinding);
    glBindVertexBuffer(kUnusedBinding, blueBuffer, 0, sizeof(GLColor));
    glEnableVertexAttribArray(colorLoc);

    // Make binding 'colorLoc' use a small buffer.
    std::vector<GLColor> greenVertices(6, GLColor::green);
    const size_t greenBufferSize = sizeof(GLColor) * 3;
    GLBuffer greenBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, greenBuffer);
    glBufferData(GL_ARRAY_BUFFER, greenBufferSize, greenVertices.data(), GL_STATIC_DRAW);
    glBindVertexBuffer(colorLoc, greenBuffer, 0, sizeof(GLColor));

    ASSERT_GL_NO_ERROR();

    // Draw without a mapped buffer. Should succeed.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Change divisor with VertexAttribDivisor. Should fail.
    glVertexAttribDivisor(colorLoc, 0);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "draw with small buffer should fail.";

    // Do a small draw. Should succeed.
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests state changes with uniform block validation.
TEST_P(WebGL2ValidationStateChangeTest, UniformBlockNegativeAPI)
{
    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
void main()
{
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform uni { vec4 vec; };
out vec4 color;
void main()
{
    color = vec;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLuint blockIndex = glGetUniformBlockIndex(program, "uni");
    ASSERT_NE(GL_INVALID_INDEX, blockIndex);

    glUniformBlockBinding(program, blockIndex, 0);

    GLBuffer uniformBuffer;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen.R, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

    const auto &quadVertices = GetQuadVertices();

    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // First draw should succeed.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Change the uniform block binding. Should fail.
    glUniformBlockBinding(program, blockIndex, 1);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION) << "Invalid uniform block binding should fail";

    // Reset to a correct state.
    glUniformBlockBinding(program, blockIndex, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Change the buffer binding. Should fail.
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION) << "Setting invalid uniform buffer should fail";

    // Reset to a correct state.
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Resize the buffer to be too small. Should fail.
    glBufferData(GL_UNIFORM_BUFFER, 1, nullptr, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Invalid buffer size should fail";
}

// Tests various state change effects on draw framebuffer validation.
TEST_P(WebGL2ValidationStateChangeTest, DrawFramebufferNegativeAPI)
{
    // Set up a simple draw from a Texture to a user Framebuffer.
    GLuint program = get2DTexturedQuadProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, posLoc);

    const auto &quadVertices = GetQuadVertices();

    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * quadVertices.size(), quadVertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    constexpr size_t kSize = 2;

    GLTexture colorBufferTexture;
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture,
                           0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    std::vector<GLColor> greenColor(kSize * kSize, GLColor::green);

    GLTexture greenTexture;
    glBindTexture(GL_TEXTURE_2D, greenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenColor.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Second framebuffer with a feedback loop. Initially unbound.
    GLFramebuffer loopedFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, loopedFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, greenTexture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Create a rendering feedback loop. Should fail.
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Reset to a valid state.
    glBindTexture(GL_TEXTURE_2D, greenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Bind a second framebuffer with a feedback loop.
    glBindFramebuffer(GL_FRAMEBUFFER, loopedFramebuffer);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Update the framebuffer texture attachment. Should succeed.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture,
                           0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests various state change effects on draw framebuffer validation with MRT.
TEST_P(WebGL2ValidationStateChangeTest, MultiAttachmentDrawFramebufferNegativeAPI)
{
    // Crashes on 64-bit Android.  http://anglebug.com/3878
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsAndroid());

    // Set up a program that writes to two outputs: one int and one float.
    constexpr char kVS[] = R"(#version 300 es
layout(location = 0) in vec2 position;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in vec2 texCoord;
layout(location = 0) out vec4 outFloat;
layout(location = 1) out uvec4 outInt;
void main()
{
    outFloat = vec4(0, 1, 0, 1);
    outInt = uvec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    constexpr GLint kPosLoc = 0;

    const auto &quadVertices = GetQuadVertices();

    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * quadVertices.size(), quadVertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kPosLoc);

    constexpr size_t kSize = 2;

    GLFramebuffer floatFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, floatFramebuffer);

    GLTexture floatTextures[2];
    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, floatTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                               floatTextures[i], 0);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLFramebuffer intFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, intFramebuffer);

    GLTexture intTextures[2];
    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, intTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, kSize, kSize, 0, GL_RGBA_INTEGER,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                               intTextures[i], 0);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    constexpr GLenum kColor0Enabled[]     = {GL_COLOR_ATTACHMENT0, GL_NONE};
    constexpr GLenum kColor1Enabled[]     = {GL_NONE, GL_COLOR_ATTACHMENT1};
    constexpr GLenum kColor0And1Enabled[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

    // Draw float. Should work.
    glBindFramebuffer(GL_FRAMEBUFFER, floatFramebuffer);
    glDrawBuffers(2, kColor0Enabled);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR() << "Draw to float texture with correct mask";
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Set an invalid component write.
    glDrawBuffers(2, kColor0And1Enabled);
    ASSERT_GL_NO_ERROR() << "Draw to float texture with invalid mask";
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // Set all 4 channels of color mask to false. Validate success.
    glColorMask(false, false, false, false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
    glColorMask(false, true, false, false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glColorMask(true, true, true, true);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Restore state.
    glDrawBuffers(2, kColor0Enabled);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR() << "Draw to float texture with correct mask";
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Bind an invalid framebuffer. Validate failure.
    glBindFramebuffer(GL_FRAMEBUFFER, intFramebuffer);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Draw to int texture with default mask";

    // Set draw mask to a valid mask. Validate success.
    glDrawBuffers(2, kColor1Enabled);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR() << "Draw to int texture with correct mask";
}

// Tests negative API state change cases with Transform Feedback bindings.
TEST_P(WebGL2ValidationStateChangeTest, TransformFeedbackNegativeAPI)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOSX());

    // TODO(anglebug.com/4533) This fails after the upgrade to the 26.20.100.7870 driver.
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform block { vec4 color; };
out vec4 colorOut;
void main()
{
    colorOut = color;
})";

    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, essl3_shaders::vs::Simple(), kFS, tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);
    glUseProgram(program);

    std::vector<Vector4> positionData;
    for (const Vector3 &quadVertex : GetQuadVertices())
    {
        positionData.emplace_back(quadVertex.x(), quadVertex.y(), quadVertex.z(), 1.0f);
    }

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(Vector4), positionData.data(),
                 GL_STATIC_DRAW);

    GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);

    glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    EXPECT_GL_NO_ERROR();

    // Set up transform feedback.
    GLTransformFeedback transformFeedback;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);

    constexpr size_t kTransformFeedbackSize = 6 * sizeof(Vector4);

    GLBuffer transformFeedbackBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, kTransformFeedbackSize * 2, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transformFeedbackBuffer);

    // Set up uniform buffer.
    GLBuffer uniformBuffer;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen.R, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

    ASSERT_GL_NO_ERROR();

    // Do the draw operation. Should succeed.
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    const GLvoid *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, kTransformFeedbackSize, GL_MAP_READ_BIT);
    ASSERT_GL_NO_ERROR();
    ASSERT_NE(nullptr, mapPointer);
    const Vector4 *typedMapPointer = reinterpret_cast<const Vector4 *>(mapPointer);
    std::vector<Vector4> actualData(typedMapPointer, typedMapPointer + 6);
    EXPECT_EQ(positionData, actualData);
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    // Draw once to update validation cache.
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Bind transform feedback buffer to another binding point. Should cause a conflict.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transformFeedbackBuffer);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Simultaneous element buffer binding should fail";

    // Reset to valid state.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    // Simultaneous non-vertex-array binding. Should fail.
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    glBindBuffer(GL_PIXEL_PACK_BUFFER, transformFeedbackBuffer);
    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Simultaneous pack buffer binding should fail";
}

// Test sampler format validation caching works.
TEST_P(WebGL2ValidationStateChangeTest, SamplerFormatCache)
{
    // Crashes in depth data upload due to lack of support for GL_UNSIGNED_INT data when
    // DEPTH_COMPONENT24 is emulated with D32_S8X24.  http://anglebug.com/3880
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsWindows() && IsAMD());

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform sampler2D sampler;
out vec4 colorOut;
void main()
{
    colorOut = texture(sampler, vec2(0));
})";

    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());

    const auto &quadVertices = GetQuadVertices();

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);

    GLint samplerLoc = glGetUniformLocation(program, "sampler");
    ASSERT_NE(-1, samplerLoc);
    glUniform1i(samplerLoc, 0);

    GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // TexImage2D should update the sampler format cache.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 2, 2, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
                 colors.data());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Sampling integer texture with a float sampler.";

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 2, 2, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, colors.data());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR() << "Depth texture with no compare mode.";

    // TexParameteri should update the sampler format cache.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Depth texture with compare mode set.";
}

// Tests that we retain the correct draw mode settings with transform feedback changes.
TEST_P(ValidationStateChangeTest, TransformFeedbackDrawModes)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOSX());

    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program, essl3_shaders::vs::Simple(),
                                        essl3_shaders::fs::Red(), tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);
    glUseProgram(program);

    std::vector<Vector4> positionData;
    for (const Vector3 &quadVertex : GetQuadVertices())
    {
        positionData.emplace_back(quadVertex.x(), quadVertex.y(), quadVertex.z(), 1.0f);
    }

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, positionData.size() * sizeof(Vector4), positionData.data(),
                 GL_STATIC_DRAW);

    GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);

    glVertexAttribPointer(positionLoc, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    // Set up transform feedback.
    GLTransformFeedback transformFeedback;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);

    constexpr size_t kTransformFeedbackSize = 6 * sizeof(Vector4);

    GLBuffer transformFeedbackBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, kTransformFeedbackSize * 2, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transformFeedbackBuffer);

    GLTransformFeedback pointsXFB;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, pointsXFB);
    GLBuffer pointsXFBBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, pointsXFBBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, pointsXFBBuffer);

    // Begin TRIANGLES, switch to paused POINTS, should be valid.
    glBeginTransformFeedback(GL_POINTS);
    glPauseTransformFeedback();
    ASSERT_GL_NO_ERROR() << "Starting point transform feedback should succeed";

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR() << "Triangle rendering should succeed";
    glDrawArrays(GL_POINTS, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Point rendering should fail";
    glDrawArrays(GL_LINES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Lines rendering should fail";
    glPauseTransformFeedback();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, pointsXFB);
    glResumeTransformFeedback();
    glDrawArrays(GL_POINTS, 0, 6);
    EXPECT_GL_NO_ERROR() << "Point rendering should succeed";
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Triangle rendering should fail";
    glDrawArrays(GL_LINES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Lines rendering should fail";

    glEndTransformFeedback();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR() << "Ending transform feeback should pass";

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDrawArrays(GL_POINTS, 0, 6);
    EXPECT_GL_NO_ERROR() << "Point rendering should succeed";
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR() << "Triangle rendering should succeed";
    glDrawArrays(GL_LINES, 0, 6);
    EXPECT_GL_NO_ERROR() << "Line rendering should succeed";
}

// Tests a valid rendering setup with two textures. Followed by a draw with conflicting samplers.
TEST_P(ValidationStateChangeTest, TextureConflict)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_storage"));

    GLint maxTextures = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextures);
    ANGLE_SKIP_TEST_IF(maxTextures < 2);

    // Set up state.
    constexpr GLint kSize = 2;

    std::vector<GLColor> greenData(4, GLColor::green);

    GLTexture textureA;
    glBindTexture(GL_TEXTURE_2D, textureA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE1);

    GLTexture textureB;
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureB);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, greenData.data());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    constexpr char kVS[] = R"(attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(varying mediump vec2 texCoord;
uniform sampler2D texA;
uniform samplerCube texB;
void main()
{
    gl_FragColor = texture2D(texA, texCoord) + textureCube(texB, vec3(1, 0, 0));
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    const auto &quadVertices = GetQuadVertices();

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);

    GLint positionLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLoc);

    GLint texALoc = glGetUniformLocation(program, "texA");
    ASSERT_NE(-1, texALoc);

    GLint texBLoc = glGetUniformLocation(program, "texB");
    ASSERT_NE(-1, texBLoc);

    glUniform1i(texALoc, 0);
    glUniform1i(texBLoc, 1);

    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    ASSERT_GL_NO_ERROR();

    // First draw. Should succeed.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Second draw to ensure all state changes are flushed.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Make the uniform use an invalid texture binding.
    glUniform1i(texBLoc, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests that mapping the element array buffer triggers errors.
TEST_P(ValidationStateChangeTest, MapElementArrayBuffer)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(program);

    std::array<GLushort, 6> quadIndices = GetQuadIndices();
    std::array<Vector3, 4> quadVertices = GetIndexedQuadVertices();

    GLsizei elementBufferSize = sizeof(quadIndices[0]) * quadIndices.size();

    GLBuffer elementArrayBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementBufferSize, quadIndices.data(), GL_STATIC_DRAW);

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices[0]) * quadVertices.size(),
                 quadVertices.data(), GL_STATIC_DRAW);

    GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    void *ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, elementBufferSize, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, ptr);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests that deleting a non-active texture does not reset the current texture cache.
TEST_P(SimpleStateChangeTest, DeleteNonActiveTextureThenDraw)
{
    constexpr char kFS[] =
        "uniform sampler2D us; void main() { gl_FragColor = texture2D(us, vec2(0)); }";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLint loc = glGetUniformLocation(program, "us");
    ASSERT_EQ(0, loc);

    auto quadVertices = GetQuadVertices();
    GLint posLoc      = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_EQ(0, posLoc);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(quadVertices[0]),
                 quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    constexpr size_t kSize = 2;
    std::vector<GLColor> red(kSize * kSize, GLColor::red);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glUniform1i(loc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Deleting TEXTURE_CUBE_MAP[0] should not affect TEXTURE_2D[0].
    GLTexture tex2;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex2);
    tex2.reset();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Deleting TEXTURE_2D[0] should start "sampling" from the default/zero texture.
    tex.reset();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

// Tests that deleting a texture successfully binds the zero texture.
TEST_P(SimpleStateChangeTest, DeleteTextureThenDraw)
{
    constexpr char kFS[] =
        "uniform sampler2D us; void main() { gl_FragColor = texture2D(us, vec2(0)); }";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLint loc = glGetUniformLocation(program, "us");
    ASSERT_EQ(0, loc);

    auto quadVertices = GetQuadVertices();
    GLint posLoc      = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_EQ(0, posLoc);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(quadVertices[0]),
                 quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    constexpr size_t kSize = 2;
    std::vector<GLColor> red(kSize * kSize, GLColor::red);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glUniform1i(loc, 1);
    tex.reset();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

void SimpleStateChangeTest::bindTextureToFbo(GLFramebuffer &fbo, GLTexture &texture)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

void SimpleStateChangeTest::drawToFboWithCulling(const GLenum frontFace, bool earlyFrontFaceDirty)
{
    // Render to an FBO
    GLFramebuffer fbo1;
    GLTexture texture1;

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    bindTextureToFbo(fbo1, texture1);

    // Clear the surface FBO to initialize it to a known value
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(GLColor::red.R, GLColor::red.G, GLColor::red.B, GLColor::red.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    glFlush();

    // Draw to FBO 1 to initialize it to a known value
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);

    if (earlyFrontFaceDirty)
    {
        glEnable(GL_CULL_FACE);
        // Make sure we don't cull
        glCullFace(frontFace == GL_CCW ? GL_BACK : GL_FRONT);
        glFrontFace(frontFace);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    glUseProgram(greenProgram);
    drawQuad(greenProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Draw into FBO 0 using FBO 1's texture to determine if culling is working or not
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glCullFace(GL_BACK);
    if (!earlyFrontFaceDirty)
    {
        // Set the culling we want to test
        glEnable(GL_CULL_FACE);
        glFrontFace(frontFace);
    }

    glUseProgram(textureProgram);
    drawQuad(textureProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();

    if (frontFace == GL_CCW)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Validates if culling rasterization states work with FBOs using CCW winding.
TEST_P(SimpleStateChangeTest, FboEarlyCullFaceBackCCWState)
{
    drawToFboWithCulling(GL_CCW, true);
}

// Validates if culling rasterization states work with FBOs using CW winding.
TEST_P(SimpleStateChangeTest, FboEarlyCullFaceBackCWState)
{
    drawToFboWithCulling(GL_CW, true);
}

TEST_P(SimpleStateChangeTest, FboLateCullFaceBackCCWState)
{
    drawToFboWithCulling(GL_CCW, false);
}

// Validates if culling rasterization states work with FBOs using CW winding.
TEST_P(SimpleStateChangeTest, FboLateCullFaceBackCWState)
{
    drawToFboWithCulling(GL_CW, false);
}

// Test that vertex attribute translation is still kept after binding it to another buffer then
// binding back to the previous buffer.
TEST_P(SimpleStateChangeTest, RebindTranslatedAttribute)
{
    // http://anglebug.com/5379
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    constexpr char kVS[] = R"(attribute vec4 a_position;
attribute float a_attrib;
varying float v_attrib;
void main()
{
    v_attrib = a_attrib;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float v_attrib;
void main()
{
    gl_FragColor = vec4(v_attrib, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_attrib");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Set up color data so red is drawn
    std::vector<GLushort> data(1000, 0xffff);

    GLBuffer redBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, redBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort) * data.size(), data.data(), GL_STATIC_DRAW);
    // Use offset not multiple of 4 GLushorts, this could force vertex translation in Metal backend.
    glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 0,
                          reinterpret_cast<const void *>(sizeof(GLushort) * 97));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(1);

    drawQuad(program, "a_position", 0.5f);
    // Verify red was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    // Verify that green was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Bind black color buffer to the same attribute with zero offset
    std::vector<GLfloat> black(6, 0.0f);
    GLBuffer blackBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, blackBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * black.size(), black.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);

    drawQuad(program, "a_position", 0.5f);
    // Verify black was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Rebind the old buffer & offset
    glBindBuffer(GL_ARRAY_BUFFER, redBuffer);
    // Use offset not multiple of 4 GLushorts
    glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 0,
                          reinterpret_cast<const void *>(sizeof(GLushort) * 97));

    drawQuad(program, "a_position", 0.5f);
    // Verify red was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that switching between programs that only contain default uniforms is correct.
TEST_P(SimpleStateChangeTest, TwoProgramsWithOnlyDefaultUniforms)
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
varying float v_attrib;
uniform float u_value;
void main()
{
    v_attrib = u_value;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float v_attrib;
void main()
{
    gl_FragColor = vec4(v_attrib, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program1, kVS, kFS);
    ANGLE_GL_PROGRAM(program2, kVS, kFS);

    // Don't use drawQuad so there's no state changes between the draw calls other than the program
    // binding.

    constexpr size_t kProgramCount = 2;
    GLuint programs[kProgramCount] = {program1, program2};
    for (size_t i = 0; i < kProgramCount; ++i)
    {
        glUseProgram(programs[i]);
        GLint uniformLoc = glGetUniformLocation(programs[i], "u_value");
        ASSERT_NE(uniformLoc, -1);

        glUniform1f(uniformLoc, static_cast<float>(i + 1) / static_cast<float>(kProgramCount));

        // Ensure position is at location 0 in both programs.
        GLint positionLocation = glGetAttribLocation(programs[i], "a_position");
        ASSERT_EQ(positionLocation, 0);
    }
    ASSERT_GL_NO_ERROR();

    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(0);

    // Draw once with each so their uniforms are updated.
    // The first draw will clear the screen to 255, 0, 0, 255
    glUseProgram(program2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // The second draw will clear the screen to 127, 0, 0, 255
    glUseProgram(program1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw with the previous program again, to make sure its default uniforms are bound again.
    glUseProgram(program2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Verify red was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that glDrawArrays when an empty-sized element array buffer is bound doesn't crash.
// Regression test for crbug.com/1172577.
TEST_P(SimpleStateChangeTest, DrawArraysWithZeroSizedElementArrayBuffer)
{
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validates GL_RASTERIZER_DISCARD state is tracked correctly
TEST_P(SimpleStateChangeTestES3, RasterizerDiscardState)
{
    glClearColor(GLColor::red.R, GLColor::red.G, GLColor::red.B, GLColor::red.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // The drawQuad() should have no effect with GL_RASTERIZER_DISCARD enabled
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(greenProgram);
    drawQuad(greenProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // The drawQuad() should draw something with GL_RASTERIZER_DISCARD disabled
    glDisable(GL_RASTERIZER_DISCARD);
    glUseProgram(greenProgram);
    drawQuad(greenProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // The drawQuad() should have no effect with GL_RASTERIZER_DISCARD enabled
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(blueProgram);
    drawQuad(blueProgram.get(), std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

class ImageRespecificationTest : public ANGLETest
{
  protected:
    ImageRespecificationTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");
        ASSERT_NE(-1, mTextureUniformLocation);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
    }

    template <typename T>
    void init2DSourceTexture(GLenum internalFormat,
                             GLenum dataFormat,
                             GLenum dataType,
                             const T *data)
    {
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void attachTargetTextureToFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTargetTexture,
                               0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderToTargetTexture()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mSourceTexture);

        glUseProgram(mProgram);
        glUniform1i(mTextureUniformLocation, 0);

        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void renderToDefaultFramebuffer(GLColor *expectedData)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(mProgram);
        glBindTexture(GL_TEXTURE_2D, mTargetTexture);
        glUniform1i(mTextureUniformLocation, 0);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, *expectedData);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    GLuint mProgram;
    GLint mTextureUniformLocation;

    GLTexture mSourceTexture;
    GLTexture mTargetTexture;

    GLFramebuffer mFramebuffer;
};

// Verify that a swizzle on an active sampler is handled appropriately
TEST_P(ImageRespecificationTest, Swizzle)
{
    GLubyte data[] = {1, 64, 128, 200};
    GLColor expectedData(data[0], data[1], data[2], data[3]);

    // Create the source and target texture
    init2DSourceTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create a framebuffer and the target texture is attached to the framebuffer.
    attachTargetTextureToFramebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render content of source texture to target texture
    // This command triggers the creation of -
    //     - draw imageviews of the texture
    //     - VkFramebuffer object of the framebuffer
    renderToTargetTexture();

    // This swizzle operation should cause the read imageviews of the texture to be released
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Draw using the newly created read imageviews
    renderToDefaultFramebuffer(&expectedData);

    // Render content of source texture to target texture, again
    renderToTargetTexture();

    // Make sure the content rendered to target texture is correct
    renderToDefaultFramebuffer(&expectedData);
}

// Verify that when a texture is respecified through glEGLImageTargetTexture2DOES,
// the Framebuffer that has the texture as a color attachment is recreated before next use.
TEST_P(ImageRespecificationTest, ImageTarget2DOESSwitch)
{
    // This is the specific problem on the Vulkan backend and needs some extensions
    ANGLE_SKIP_TEST_IF(
        !IsGLExtensionEnabled("GL_OES_EGL_image_external") ||
        !IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_gl_texture_2D_image"));

    GLubyte data[] = {1, 64, 128, 200};
    GLColor expectedData(data[0], data[1], data[2], data[3]);

    // Create the source texture
    init2DSourceTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Create the first EGL image to attach the framebuffer through the texture
    GLTexture firstTexture;

    glBindTexture(GL_TEXTURE_2D, firstTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    EGLWindow *window = getEGLWindow();
    EGLint attribs[]  = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR firstEGLImage = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(firstTexture.get())), attribs);
    ASSERT_EGL_SUCCESS();

    // Create the target texture and attach it to the framebuffer
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, firstEGLImage);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    attachTargetTextureToFramebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render content of source texture to target texture
    // This command triggers the creation of -
    //     - draw imageviews of the texture
    //     - VkFramebuffer object of the framebuffer
    renderToTargetTexture();

    // Make sure the content rendered to target texture is correct
    renderToDefaultFramebuffer(&expectedData);

    // Create the second EGL image
    GLTexture secondTexture;

    glBindTexture(GL_TEXTURE_2D, secondTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    EGLImageKHR secondEGLImage = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(secondTexture.get())), attribs);
    ASSERT_EGL_SUCCESS();

    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    // This will release all the imageviews related to the first EGL image
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, secondEGLImage);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the first EGL image to the target texture again
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, firstEGLImage);

    // This is for checking this code can deal with the problem even if both ORPHAN and
    // COLOR_ATTACHMENT dirty bits are set.
    GLTexture tempTexture;

    glBindTexture(GL_TEXTURE_2D, tempTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // This sets COLOR_ATTACHMENT dirty bit
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // The released imageviews related to "secondEGLImage" will be garbage collected
    renderToDefaultFramebuffer(&expectedData);

    // Process both OPPHAN and COLOR_ATTACHMENT dirty bits
    renderToTargetTexture();

    // Make sure the content rendered to target texture is correct
    renderToDefaultFramebuffer(&expectedData);

    // Render content of source texture to target texture
    attachTargetTextureToFramebuffer();
    renderToTargetTexture();

    // Make sure the content rendered to target texture is correct
    renderToDefaultFramebuffer(&expectedData);

    eglDestroyImageKHR(window->getDisplay(), firstEGLImage);
    eglDestroyImageKHR(window->getDisplay(), secondEGLImage);
}

// Covers a bug where sometimes we wouldn't catch invalid element buffer sizes.
TEST_P(WebGL2ValidationStateChangeTest, DeleteElementArrayBufferValidation)
{
    GLushort indexData[] = {0, 1, 2, 3};

    GLBuffer elementArrayBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Zero(), essl1_shaders::fs::Red());
    glUseProgram(program);

    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_SHORT, 0);

    elementArrayBuffer.reset();

    // Must use a non-0 offset and a multiple of the type size.
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_SHORT, reinterpret_cast<const void *>(0x4));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Covers a bug in the D3D11 back-end related to how buffers are translated.
TEST_P(RobustBufferAccessWebGL2ValidationStateChangeTest, BindZeroSizeBufferThenDeleteBufferBug)
{
    // SwiftShader does not currently support robustness.
    ANGLE_SKIP_TEST_IF(isSwiftshader());

    // http://anglebug.com/4872
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsVulkan());

    // no intent to follow up on this failure.
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // no intent to follow up on this failure.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    // no intent to follow up on this failure.
    ANGLE_SKIP_TEST_IF(IsOSX());

    // Mali does not support robustness now.
    ANGLE_SKIP_TEST_IF(IsARM());

    // TODO(anglebug.com/5491)
    ANGLE_SKIP_TEST_IF(IsIOS() && IsOpenGLES());

    std::vector<GLubyte> data(48, 1);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());
    glUseProgram(program);

    // First bind and draw with a buffer with a format we know to be "Direct" in D3D11.
    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, 48, data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Then bind a zero size buffer and draw.
    GLBuffer secondBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, secondBuffer);
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 1, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Finally delete the original buffer. This triggers the bug.
    arrayBuffer.reset();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
}

// Tests DrawElements with an empty buffer using a VAO.
TEST_P(WebGL2ValidationStateChangeTest, DrawElementsEmptyVertexArray)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glUseProgram(program);

    // Draw with empty buffer. Out of range but valid.
    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glDrawElements(GL_TRIANGLES, 0, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(0x1000));

    // Switch VAO. No buffer bound, should be an error.
    GLVertexArray vao;
    glBindVertexArray(vao);
    glDrawElements(GL_LINE_STRIP, 0x1000, GL_UNSIGNED_SHORT,
                   reinterpret_cast<const GLvoid *>(0x1000));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that closing the render pass due to an update to UBO data then drawing non-indexed followed
// by indexed works.
TEST_P(SimpleStateChangeTestES31, DrawThenUpdateUBOThenDrawThenDrawIndexed)
{
    // First, create the index buffer and issue an indexed draw call.  This clears the index dirty
    // bit.
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    const std::array<GLuint, 4> kIndexData = {0, 1, 2, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexData), kIndexData.data(), GL_STATIC_DRAW);

    // Setup vertices.
    const std::array<GLfloat, 6> kVertices = {
        -1, -1, 3, -1, -1, 3,
    };
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Create a uniform buffer that will get modified.  This is used to break the render pass.
    const std::array<GLuint, 4> kUboData1 = {0x12345678u, 0, 0, 0};

    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kUboData1), kUboData1.data(), GL_DYNAMIC_DRAW);

    // Set up a program.  The same program is used for all draw calls to avoid state change due to
    // program change.
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform block { uint data; } ubo;
uniform uint expect;
uniform vec4 successColor;
out vec4 colorOut;
void main()
{
    colorOut = ubo.data == expect ? successColor : colorOut = vec4(0, 0, 0, 0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    const GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    const GLint expectLoc   = glGetUniformLocation(program, "expect");
    const GLint successLoc  = glGetUniformLocation(program, "successColor");
    ASSERT_NE(-1, positionLoc);
    ASSERT_NE(-1, expectLoc);
    ASSERT_NE(-1, successLoc);

    glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLoc);

    glUniform1ui(expectLoc, kUboData1[0]);
    glUniform4f(successLoc, 0, 0, 0, 1);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Then upload data to the UBO so on next use the render pass has to break.  This draw call is
    // not indexed.
    constexpr GLuint kUboData2 = 0x87654321u;
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(kUboData2), &kUboData2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUniform1ui(expectLoc, kUboData2);
    glUniform4f(successLoc, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Issue another draw call that is indexed.  The index buffer should be bound correctly on the
    // new render pass.
    glUniform4f(successLoc, 0, 0, 1, 0);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Ensure correct rendering.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
}

// Test that switching framebuffers then a non-indexed draw followed by an indexed one works.
TEST_P(SimpleStateChangeTestES31, DrawThenChangeFBOThenDrawThenDrawIndexed)
{
    // Create a framebuffer, and make sure it and the default framebuffer are fully synced.
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    GLFramebuffer fbo;
    GLTexture texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 16, 16);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // First, create the index buffer and issue an indexed draw call.  This clears the index dirty
    // bit.
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    const std::array<GLuint, 4> kIndexData = {0, 1, 2, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexData), kIndexData.data(), GL_STATIC_DRAW);

    // Setup vertices.
    const std::array<GLfloat, 6> kVertices = {
        -1, -1, 3, -1, -1, 3,
    };
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a program.  The same program is used for all draw calls to avoid state change due to
    // program change.
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform vec4 colorIn;
out vec4 colorOut;
void main()
{
    colorOut = colorIn;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    const GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    const GLint colorLoc    = glGetUniformLocation(program, "colorIn");
    ASSERT_NE(-1, positionLoc);
    ASSERT_NE(-1, colorLoc);

    glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLoc);

    glUniform4f(colorLoc, 1, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Then switch to fbo and issue a non-indexed draw call followed by an indexed one.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUniform4f(colorLoc, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUniform4f(colorLoc, 0, 0, 1, 0);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Ensure correct rendering.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that switching framebuffers then a non-indexed draw followed by an indexed one works, with
// another context flushing work in between the two draw calls.
TEST_P(SimpleStateChangeTestES31, DrawThenChangeFBOThenDrawThenFlushInAnotherThreadThenDrawIndexed)
{
    // Create a framebuffer, and make sure it and the default framebuffer are fully synced.
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    GLFramebuffer fbo;
    GLTexture texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 16, 16);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // First, create the index buffer and issue an indexed draw call.  This clears the index dirty
    // bit.
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    const std::array<GLuint, 4> kIndexData = {0, 1, 2, 0};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexData), kIndexData.data(), GL_STATIC_DRAW);

    // Setup vertices.
    const std::array<GLfloat, 6> kVertices = {
        -1, -1, 3, -1, -1, 3,
    };
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a program.  The same program is used for all draw calls to avoid state change due to
    // program change.
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform vec4 colorIn;
out vec4 colorOut;
void main()
{
    colorOut = colorIn;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    const GLint positionLoc = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    const GLint colorLoc    = glGetUniformLocation(program, "colorIn");
    ASSERT_NE(-1, positionLoc);
    ASSERT_NE(-1, colorLoc);

    glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLoc);

    glUniform4f(colorLoc, 0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Then switch to fbo and issue a non-indexed draw call followed by an indexed one.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUniform4f(colorLoc, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // In between the two calls, make sure the first render pass is submitted, so the primary
    // command buffer is reset.
    {
        EGLWindow *window          = getEGLWindow();
        EGLDisplay dpy             = window->getDisplay();
        EGLConfig config           = window->getConfig();
        EGLint pbufferAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE};
        EGLSurface surface         = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
        EGLContext ctx             = window->createContext(EGL_NO_CONTEXT, nullptr);
        EXPECT_EGL_SUCCESS();
        std::thread flushThread = std::thread([&]() {
            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
            EXPECT_EGL_SUCCESS();

            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_GL_NO_ERROR();

            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        });
        flushThread.join();

        eglDestroySurface(dpy, surface);
        eglDestroyContext(dpy, ctx);
    }

    glUniform4f(colorLoc, 0, 0, 1, 0);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    // Ensure correct rendering.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
}

// Negative test for EXT_primitive_bounding_box
TEST_P(SimpleStateChangeTestES31, PrimitiveBoundingBoxNegativeTest)
{
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_EXT_primitive_bounding_box"));

    glPrimitiveBoundingBoxEXT(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLfloat boundingBox[8] = {0};
    glGetFloatv(GL_PRIMITIVE_BOUNDING_BOX_EXT, boundingBox);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Update an element array buffer that is already in use.
TEST_P(SimpleStateChangeTest, UpdateBoundElementArrayBuffer)
{
    constexpr char kVS[] = R"(attribute vec4 position;
attribute float color;
varying float colorVarying;
void main()
{
    gl_Position = position;
    colorVarying = color;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float colorVarying;
void main()
{
    if (colorVarying == 1.0)
    {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    glUseProgram(testProgram);

    GLint posLoc = glGetAttribLocation(testProgram, "position");
    ASSERT_NE(-1, posLoc);
    GLint colorLoc = glGetAttribLocation(testProgram, "color");
    ASSERT_NE(-1, colorLoc);

    std::array<GLushort, 6> quadIndices = GetQuadIndices();

    std::vector<GLubyte> indices;
    for (GLushort index : quadIndices)
    {
        indices.push_back(static_cast<GLubyte>(index));
    }

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(),
                 GL_STATIC_DRAW);

    std::array<Vector3, 4> quadVertices = GetIndexedQuadVertices();

    std::vector<Vector3> positionVertices;
    for (Vector3 vertex : quadVertices)
    {
        positionVertices.push_back(vertex);
    }
    for (Vector3 vertex : quadVertices)
    {
        positionVertices.push_back(vertex);
    }

    GLBuffer positionVertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, positionVertices.size() * sizeof(positionVertices[0]),
                 positionVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    std::vector<float> colorVertices = {1, 1, 1, 1, 0, 0, 0, 0};

    GLBuffer colorVertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, colorVertices.size() * sizeof(colorVertices[0]),
                 colorVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(colorLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    indices.clear();
    for (GLushort index : quadIndices)
    {
        indices.push_back(static_cast<GLubyte>(index + 4));
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(indices[0]),
                    indices.data());

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Covers a bug where we would use a stale cache variable in the Vulkan back-end.
TEST_P(SimpleStateChangeTestES3, DeleteFramebufferBeforeQuery)
{
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Zero(), essl1_shaders::fs::Red());
    glUseProgram(testProgram);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, 16, 16);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLfloat floatArray[] = {1, 2, 3, 4};

    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * 4, floatArray, GL_DYNAMIC_COPY);
    glDrawElements(GL_TRIANGLE_FAN, 5, GL_UNSIGNED_SHORT, 0);

    fbo.reset();

    GLQuery query2;
    glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, query2);

    ASSERT_GL_NO_ERROR();
}

// Covers an edge case
TEST_P(SimpleStateChangeTestES3, TextureTypeConflictAfterDraw)
{
    constexpr char kVS[] = R"(precision highp float;
attribute vec4 a_position;
void main()
{
    gl_Position = a_position;
}
)";

    constexpr char kFS[] = R"(precision highp float;
uniform sampler2D u_2d1;
uniform samplerCube u_Cube1;
void main()
{
    gl_FragColor = texture2D(u_2d1, vec2(0.0, 0.0)) + textureCube(u_Cube1, vec3(0.0, 0.0, 0.0));
}
)";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    glUseProgram(testProgram);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_SRGB8, 1268, 614);

    GLint uniformloc = glGetUniformLocation(testProgram, "u_Cube1");
    ASSERT_NE(-1, uniformloc);
    glUniform1i(uniformloc, 1);

    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // Trigger the state update.
    glLinkProgram(testProgram);
    texture.reset();

    // The texture types are now conflicting, and draws should fail.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Regression test for a bug where a mutable texture is used with non-zero base level then rebased
// to zero but made incomplete and attached to the framebuffer.  The texture's image is not
// recreated with level 0, leading to errors when drawing to the framebuffer.
TEST_P(SimpleStateChangeTestES3, NonZeroBaseMutableTextureThenZeroBaseButIncompleteBug)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    const std::array<GLColor, 4> kMip0Data = {GLColor::red, GLColor::red, GLColor::red,
                                              GLColor::red};
    const std::array<GLColor, 2> kMip1Data = {GLColor::green, GLColor::green};

    // Create two textures.
    GLTexture immutableTex;
    glBindTexture(GL_TEXTURE_2D, immutableTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kMip0Data.data());

    GLTexture mutableTex;
    glBindTexture(GL_TEXTURE_2D, mutableTex);
    for (uint32_t mip = 0; mip < 2; ++mip)
    {
        const uint32_t size = 4 >> mip;
        glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA8, size, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     mip == 0 ? kMip0Data.data() : kMip1Data.data());
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Sample from the mutable texture at non-zero base level.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, immutableTex, 0);
    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMip1Data[0]);

    // Rebase the mutable texture to zero, but enable mipmapping which makes it incomplete.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Remove feedback loop for good measure.
    glBindTexture(GL_TEXTURE_2D, immutableTex);

    // Draw into base zero of the texture.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mutableTex, 0);
    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.0f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, kMip1Data[1]);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a bug where the framebuffer binding was not synced during invalidate when a
// clear operation was deferred.
TEST_P(SimpleStateChangeTestES3, ChangeFramebufferThenInvalidateWithClear)
{
    // Clear the default framebuffer.
    glClear(GL_COLOR_BUFFER_BIT);

    // Start rendering to another framebuffer
    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Switch back to the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Invalidate it.  Don't invalidate color, as that's the one being cleared.
    constexpr GLenum kAttachment = GL_DEPTH;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &kAttachment);
    EXPECT_GL_NO_ERROR();
}

// Test that clear / invalidate / clear works.  The invalidate is for a target that's not cleared.
// Regression test for a bug where invalidate() would start a render pass to perform the first
// clear, while the second clear didn't expect a render pass opened without any draw calls in it.
TEST_P(SimpleStateChangeTestES3, ClearColorInvalidateDepthClearColor)
{
    // Clear color.
    glClear(GL_COLOR_BUFFER_BIT);

    // Invalidate depth.
    constexpr GLenum kAttachment = GL_DEPTH;
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, &kAttachment);
    EXPECT_GL_NO_ERROR();

    // Clear color again.
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a bug where glInvalidateFramebuffer(GL_FRAMEBUFFER, ...) was invalidating
// both the draw and read framebuffers.
TEST_P(SimpleStateChangeTestES3, InvalidateFramebufferShouldntInvalidateReadFramebuffer)
{
    // Create an invalid read framebuffer.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, 1, 1, 1);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, tex, 0, 5);

    // Invalidate using GL_FRAMEBUFFER.  If GL_READ_FRAMEBUFFER was used, validation would fail due
    // to the framebuffer not being complete.  A bug here was attempting to invalidate the read
    // framebuffer given GL_FRAMEBUFFER anyway.
    constexpr std::array<GLenum, 2> kAttachments = {GL_DEPTH, GL_STENCIL};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, kAttachments.data());
    EXPECT_GL_NO_ERROR();
}

// Covers situations where vertex conversion could read out of bounds.
TEST_P(SimpleStateChangeTestES3, OutOfBoundsByteAttribute)
{
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(testProgram);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 2, nullptr, GL_STREAM_COPY);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_BYTE, false, 0xff, reinterpret_cast<const void *>(0xfe));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 1, 10, 1000);
}

// Test that respecifies a buffer after we start XFB.
TEST_P(SimpleStateChangeTestES3, RespecifyBufferAfterBeginTransformFeedback)
{
    std::vector<std::string> tfVaryings = {"gl_Position"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(testProgram, essl3_shaders::vs::Simple(),
                                        essl3_shaders::fs::Green(), tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);

    glUseProgram(testProgram);
    GLBuffer buffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 3 * 2 * 7, nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);
    glBeginTransformFeedback(GL_TRIANGLES);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 3 * 4 * 6, nullptr, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Regression test for a bug in the Vulkan backend where a draw-based copy after a deferred flush
// would lead to an image view being destroyed too early.
TEST_P(SimpleStateChangeTestES3, DrawFlushThenCopyTexImage)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());

    // Issue a cheap draw call and a flush
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.0f);
    glDisable(GL_SCISSOR_TEST);
    glFlush();

    constexpr GLsizei kSize = 32;

    // Then an expensive copy tex image
    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGB8, kSize, kSize, kSize);
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 0, 0, kSize, kSize);
    glFlush();
    ASSERT_GL_NO_ERROR();
}

TEST_P(SimpleStateChangeTestES3, DrawFlushThenBlit)
{
    constexpr GLsizei kSize = 256;
    const std::vector<GLColor> data(kSize * kSize, GLColor::red);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture readColor;
    glBindTexture(GL_TEXTURE_2D, readColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, readColor, 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Issue a cheap draw call and a flush
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.0f);
    glDisable(GL_SCISSOR_TEST);
    glFlush();

    // Then an expensive blit
    glBlitFramebuffer(0, 0, kSize, kSize, kSize + 2, kSize, 0, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glFlush();
    ASSERT_GL_NO_ERROR();
}

class VertexAttribArrayStateChangeTest : public ANGLETest
{
  protected:
    VertexAttribArrayStateChangeTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
attribute vec4 color;
varying vec4 colorOut;

void main()
{
    gl_Position = position;
    colorOut = color;
})";

        constexpr char kFS[] = R"(precision highp float;
varying vec4 colorOut;

void main()
{
    gl_FragColor = colorOut;
})";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mPosAttribLocation = glGetAttribLocation(mProgram, "position");
        ASSERT_NE(-1, mPosAttribLocation);
        mColorAttribLocation = glGetAttribLocation(mProgram, "color");
        ASSERT_NE(-1, mColorAttribLocation);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        ASSERT_GL_NO_ERROR();

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

        const float posAttribData[] = {
            -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(posAttribData), posAttribData, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void testTearDown() override
    {
        if (mVertexBuffer != 0)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }

        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
    }

    GLuint mProgram;
    GLint mPosAttribLocation;
    GLint mColorAttribLocation;
    GLuint mVertexBuffer;
};

TEST_P(VertexAttribArrayStateChangeTest, Basic)
{
    glUseProgram(mProgram);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    glVertexAttribPointer(mPosAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPosAttribLocation);
    glDisableVertexAttribArray(mColorAttribLocation);
    glVertexAttrib4f(mColorAttribLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    // Don't try to verify the color of this draw as red here because it might
    // hide the bug
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glVertexAttrib4f(mColorAttribLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

TEST_P(SimpleStateChangeTestES3, DepthOnlyToColorAttachmentPreservesBlendState)
{

    constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
attribute vec4 color;
varying vec4 colorOut;

void main()
{
    gl_Position = position;
    colorOut = color;
})";

    constexpr char kFS[] = R"(precision highp float;
varying vec4 colorOut;

void main()
{
    gl_FragColor = colorOut;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint posAttribLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, posAttribLocation);
    GLint colorAttribLocation = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorAttribLocation);

    GLBuffer vertexBuffer;
    GLTexture texture;
    GLFramebuffer framebuffer;
    GLRenderbuffer depthRenderbuffer;
    GLFramebuffer depthFramebuffer;

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    const float posAttribData[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(posAttribData), posAttribData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    constexpr int kWidth  = 1;
    constexpr int kHeight = 1;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kWidth, kHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthRenderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(posAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttribLocation);
    glDisableVertexAttribArray(colorAttribLocation);
    glVertexAttrib4f(colorAttribLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}

}  // anonymous namespace

ANGLE_INSTANTIATE_TEST_ES2(StateChangeTest);
ANGLE_INSTANTIATE_TEST_ES2(LineLoopStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES2(StateChangeRenderTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(StateChangeTestES3);
ANGLE_INSTANTIATE_TEST_ES3(StateChangeTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(StateChangeRenderTestES3);
ANGLE_INSTANTIATE_TEST_ES3(StateChangeRenderTestES3);

ANGLE_INSTANTIATE_TEST_ES2(SimpleStateChangeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SimpleStateChangeTestES3);
ANGLE_INSTANTIATE_TEST_ES3(SimpleStateChangeTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImageRespecificationTest);
ANGLE_INSTANTIATE_TEST_ES3(ImageRespecificationTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SimpleStateChangeTestES31);
ANGLE_INSTANTIATE_TEST_ES31(SimpleStateChangeTestES31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SimpleStateChangeTestComputeES31);
ANGLE_INSTANTIATE_TEST_ES31(SimpleStateChangeTestComputeES31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SimpleStateChangeTestComputeES31PPO);
ANGLE_INSTANTIATE_TEST_ES31(SimpleStateChangeTestComputeES31PPO);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ValidationStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES3(ValidationStateChangeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGL2ValidationStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES3(WebGL2ValidationStateChangeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RobustBufferAccessWebGL2ValidationStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES3(RobustBufferAccessWebGL2ValidationStateChangeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ValidationStateChangeTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ValidationStateChangeTestES31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGLComputeValidationStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES31(WebGLComputeValidationStateChangeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VertexAttribArrayStateChangeTest);
ANGLE_INSTANTIATE_TEST_ES3(VertexAttribArrayStateChangeTest);
