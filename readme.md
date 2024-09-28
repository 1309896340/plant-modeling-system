

### shaders

1. default: 默认管线，拥有3个属性组“位置”、“法向量”、“颜色”、“材质坐标”，带一个材质图片
2. normal: 法向量可视化管线，顶点着色器和default基本一致，添加了几何着色器将点元发射为法向量，不做其他图元的处理
3. auxiliary: 场景辅助显示元素，渲染坐标轴、标注箭头等



9月27日：
考虑封装多Mesh组合对象的代码，具体如下
1. 删除Mesh的纯虚函数update的空实现，使得Mesh成为纯虚函数，不能直接使用
2. 为Mesh实现一个迭代映射操作，对其Vertex进行变换操作，采用lambda表达式作为参数，定义为 `std::function<Vertex(Vertex,float,float)>` ，其中的两个 `float` 分别对应 `updateVertex()` 回调中的网格归一化坐标 `u,v` ，该函数命名为 `transformVertex()` （完成，未测试）
3. 声明一个不可变网格类 `FixedGeometry` 继承自 `Geometry` 并且将其 `update` 虚方法采用空实现 （完成，未测试）
4. 实现两个 `Mesh` 之间的相加运算 `FixedGeometry operator+(const Mesh &m1, const Mesh &m2)`，得到的 `FixedGeometry` 实例为两个网格的拼接，且无法对其进行更新操作

但如上所示的相加操作只能生成无法编辑的退化网格，退化为了 

考虑增加的新特性：为 `Geometry` 封装一个 `transform` 方法，与前面的 `transformVertex()` 类似，但其回调函数不会包括网格归一化坐标，回调定义为 `std::function<Vertex(Vertex)>` ，目的是为了实现基本的空间变换操作。同时这个变换操作对继承自 `Geometry` 的 `FixedGeometry` 也是合理的


问题：在实现第5步过程中，发现问题，形体由多个Mesh子类构成，Mesh子类通过继承Mesh来具体化，由于对于 `operator+()` 无法进行向上转型，因此需要靠多个重载才能将所有形体之间的相加运算完全定义
解决：将重载运算符定义为Mesh的方法，但是目前不清楚如果将另一个算子定义为 `const Mesh &m` 会不会对Mesh的子类进行“向上转型”的操作，需要测试
测试：写了一个demo来测试可行性，实测可以正常进行向上转型操作
补充想法：也许应该将Mesh的求和重载在 `FixedGeometry` 上做具体实现，而将Mesh的求和重载先将两个算子构造为 `FixedGeometry`，再将它们进行求和。同样的运算也可以对两个Geometry对象做，那么干脆将这个运算重载从Mesh中移除，放到Geometry中，这样相当于将这个运算从Geometry继承过来，而原本相加操作就是定义在vertices和surfaces这两个Geometry层面的成员上的，非常合理
问题：不该在Geometry上重载求和运算符返回 `FixedGeometry`，这么做使得Geometry依赖于FixedGeometry，是非常不合理的


