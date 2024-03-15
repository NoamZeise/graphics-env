#include "framebuffer.h"

#include <stdexcept>
#include <graphics/logger.h>
#include "ogl_helper.h"

struct InternalAttachment {
    GLuint id;
    GlFramebuffer::AttachmentType type;
    
    ~InternalAttachment() {
	switch(type) {
	case GlFramebuffer::AttachmentType::renderbuffer:
	    glDeleteRenderbuffers(1, &id);
	    break;
	case GlFramebuffer::AttachmentType::texture2D:
	    glDeleteTextures(1, &id);
	    break;
	}
    }
};

void createAttachment(
	InternalAttachment* pAttach,
	GlFramebuffer::Attachment attachBlueprint,
	GLsizei width, GLsizei height, int samples,
	std::vector<GLenum>* pDrawBuffers);

std::string framebufferError(int status);

GlFramebuffer::GlFramebuffer(GLsizei width, GLsizei height, int samples,
			 std::vector<Attachment> attachments) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    this->attachments = new InternalAttachment[attachments.size()];
    this->attachmentCount = attachments.size();
    std::vector<GLenum> drawBuffers;

    for(unsigned int i = 0; i < attachments.size(); i++)
	createAttachment(&this->attachments[i], attachments[i], width, height, samples,
	&drawBuffers);
    
    if(drawBuffers.size() > 0)
	glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    
    int framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	throw std::runtime_error("failed to create opengl framebuffer! status: "
				 + framebufferError(framebufferStatus));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GlFramebuffer::~GlFramebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    delete[] attachments;
}

GLuint GlFramebuffer::id() {
    return framebuffer;
}

GLuint GlFramebuffer::textureId(unsigned int attachmentIndex) {
    if(attachmentIndex >= this->attachmentCount)
	throw std::runtime_error("tried to access an "
				 "attachment index that was out of range");
    if(this->attachments[attachmentIndex].type != AttachmentType::texture2D)
	throw std::runtime_error("tried to get a textureId from "
				 "a non texture2D attachment");
    return this->attachments[attachmentIndex].id;
}

GLuint genRenderbuffer(int samples, int format, GLsizei width, GLsizei height) {
    GLuint rb;
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    if(samples > 1)
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
    else
	glRenderbufferStorage(GL_RENDERBUFFER, format, width, height); 
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rb;
}

void createAttachment(
	InternalAttachment* pAttach,
	GlFramebuffer::Attachment attachBlueprint,
	GLsizei width, GLsizei height, int samples,
	std::vector<GLenum>* pDrawBuffers) {
    pAttach->type = attachBlueprint.type;
    
    switch(pAttach->type) {
    case GlFramebuffer::AttachmentType::renderbuffer:
	pAttach->id = genRenderbuffer(samples, attachBlueprint.format, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, (GLenum)attachBlueprint.position,
				  GL_RENDERBUFFER, pAttach->id);
	break;
    case GlFramebuffer::AttachmentType::texture2D:
	pAttach->id = ogl_helper::genTexture(attachBlueprint.format, width, height, 0,
					   false, GL_NEAREST, GL_CLAMP_TO_BORDER, samples);
	glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum)attachBlueprint.position,
			       samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
			       pAttach->id, 0);
	pDrawBuffers->push_back((GLenum)attachBlueprint.position);
	break;
	default:
	    throw std::runtime_error("unrecognised attachment type in createAttachment switch");
    }
}

std::string framebufferError(int status) {
    switch(status) {
    case GL_FRAMEBUFFER_UNDEFINED:
	return "undefined";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	return "incomplete attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	return "incomplete draw buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	return "incomplete read buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
	return "unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
	return "multisampling incomplete";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
	return "incomplete layer targets";
    default:
	return "unknown error code";
    }
}
