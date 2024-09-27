

### shaders

1. default: 默认管线，拥有3个属性组“位置”、“法向量”、“颜色”、“材质坐标”，带一个材质图片
2. normal: 法向量可视化管线，顶点着色器和default基本一致，添加了几何着色器将点元发射为法向量，不做其他图元的处理
3. auxiliary: 场景辅助显示元素，渲染坐标轴、标注箭头等



9月27日：
考虑封装多Mesh组合对象的代码，具体如下
1. 删除Mesh的纯虚函数update的空实现，使得Mesh成为纯虚函数，不能直接使用
2. 为Mesh实现一个迭代映射操作，对其Vertex进行变换操作，采用lambda表达式作为参数，定义为 `std::function<Vertex(Vertex,float,float)>` ，其中的两个 `float` 分别对应 `updateVertex()` 回调中的网格归一化坐标 `u,v` ，该函数命名为 `transformVertex()`
3. 声明一个不可变网格类 `FixedMesh` 继承自 `Geometry` 并且将其 `update` 虚方法采用空实现
4. 实现两个 `Mesh` 之间的相加运算 `FixedMesh operator+(const Mesh &m1, const Mesh &m2)`，得到的 `FixedMesh` 实例为两个网格的拼接，且无法对其进行更新操作

但如上所示的相加操作只能生成无法编辑的退化网格，退化为了 

考虑增加的新特性：为 `Geometry` 封装一个 `transform` 方法，与前面的 `transformVertex()` 类似，但其回调函数不会包括网格归一化坐标，回调定义为 `std::function<Vertex(Vertex)>` ，目的是为了实现基本的空间变换操作。同时这个变换操作对继承自 `Geometry` 的 `FixedMesh` 也是合理的


