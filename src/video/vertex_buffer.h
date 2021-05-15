#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

class VertexBuffer {
   public:
    explicit VertexBuffer(const unsigned long);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
    void push(const float*) const;
    unsigned long size() const { return size_; };

   private:
    const unsigned int VBO_;
    const unsigned long size_;
};

#endif /* VERTEX_BUFFER_H */
