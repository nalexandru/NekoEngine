Camera
======

Renders the scene.

Creation arguments
------------------

* Active: Activate the camera after the scene loads.
* Fov: Field of view angle in degrees.
* Near: Near plane distance.
* Far: Far plane distance.
* Projection: Projection type. Valid values are Perspective, Orthographic.

Constants
---------

.. c:enum:: ProjectionType

    Camera projection type

    .. c:enumerator::
        Perspective
        Orthographic

Methods
-------
.. lua:class:: Camera

    .. lua:method:: Rotation([rotation])

        Gets or sets the rotation.

        :param rotation: Rotation in euler angles.
        :type rotation: NeVec3 or x, y, z numbers
        :return: Rotation euler angles, if called without parameters.
        :rtype: number

    .. lua:method:: FieldOfView([fov])

        Gets or sets the field of view.

        :param fov: Field of view angle in degrees.
        :type fov: number
        :return: Field of view angle in degrees, if called without parameters.
        :rtype: number

    .. lua:method:: Near([distance])

        Gets or sets the near plane distance.

        :param distance: Distance.
        :type distance: number
        :return: Near plane distance, if called without parameters.
        :rtype: number

    .. lua:method:: Far([distance])

        Gets or sets the far plane distance.

        :param distance: Distance.
        :type distance: number
        :return: Far plane distance, if called without parameters.
        :rtype: number

    .. lua:method:: ProjectionType([type])

        Gets or sets the far plane distance.

        :param type: Projection type.
        :type type: :c:enum:`ProjectionType`
        :return: Projection type, if called without parameters.
        :rtype: :c:enum:`ProjectionType`
