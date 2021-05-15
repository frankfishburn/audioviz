#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

class VertexArray {
   public:
    VertexArray();
    ~VertexArray();

    void bind() const;

   private:
    const unsigned int VAO_;
};

#endif /* VERTEX_ARRAY_H */
