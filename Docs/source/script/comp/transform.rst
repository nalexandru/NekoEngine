Transform
=========

Represents the position, rotation and scale of an entity.

Creation arguments
------------------

* Parent: Name of the parent transform.
* Position: Initial position.
* Rotation: Initial rotation.
* Scale: Initial scale.

Methods
-------
.. lua:class:: Transform

    .. lua:method:: Move(movement)

        Move the transform.

        :param movement: Movement vector.
        :type movement: NeVec3 or x, y, z numbers

    .. lua:method:: Rotate(angle, axis)

        Rotate the transform.

        :param angle: Rotation angle.
        :type angle: number
        :param axis: Rotation axis.
        :type axis: NeVec3 or x, y, z numbers

    .. lua:method:: AdjustScale(scale)

        Scale the transform.

        :param scale: Scale vector.
        :type scale: NeVec3 or x, y, z numbers

    .. lua:method:: Forward()

        Retrieve the forward vector.

        :return: Forward vector
        :rtype: NeVec3

    .. lua:method:: Right()

        Retrieve the right vector.

        :return: Right vector.
        :rtype: NeVec3

    .. lua:method:: Up()

        Retrieve the up vector.

        :return: Up vector.
        :rtype: NeVec3

    .. lua:method:: Dirty()

        Returns true if the transform was modified and needs to be updated. The update is done during ``SystemGrup.PostLogic``.

        :return: Dirty flag.
        :rtype: boolean

    .. lua:method:: MoveForward()

        Move the transform along it's forward vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: MoveBackward()

        Move the transform along it's backward vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: MoveRight()

        Move the transform along it's right vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: MoveLeft()

        Move the transform along it's left vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: MoveUp()

        Move the transform along it's up vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: MoveDown()

        Move the transform along it's down vector.

        :param movement: Movement.
        :type movement: number

    .. lua:method:: Position([position])

        Gets or sets the position.

        :param movement: Movement.
        :type movement: NeVec3 or x, y, z numbers
        :return: Position, if called with no parameters.
        :rtype: NeVec3

    .. lua:method:: WorldPosition()

        Gets the world position.

        :return: World position.
        :rtype: NeVec3

    .. lua:method:: Rotation([rotation])

        Gets or sets the rotation.

        :param movement: Movement.
        :type movement: NeQuaternion or x, y, z, w numbers
        :return: Rotation, if called with no parameters.
        :rtype: NeQuaternion

    .. lua:method:: WorldRotation()

        Gets the world rotation.

        :return: World rotation.
        :rtype: NeQuaternion

    .. lua:method:: Scale([scale])

        Gets or sets the scale.

        :param movement: Movement
        :type movement: NeVec3 or x, y, z numbers
        :return: Scale, if called with no parameters.
        :rtype: NeVec3

    .. lua:method:: WorldScale()

        Gets the world scale.

        :return: World scale.
        :rtype: NeVec3
