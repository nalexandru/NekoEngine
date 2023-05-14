Light
=====



Constants
---------

.. c:enum:: LightType

    Light type

    .. c:enumerator::
        Directional
        Point
        Spot

Methods
-------
.. lua:class:: Light

    .. lua:method:: Color()

        Retrieve the time elapsed since the engine was started.

        :return: Time elapsed.
        :rtype: integer

    .. lua:method:: Intensity()
    .. lua:method:: InnerRadius()
    .. lua:method:: OuterRadius()
    .. lua:method:: InnerCutoff()
    .. lua:method:: OuterCutoff()
    .. lua:method:: Type()

    :c:enum:`ProjectionType`
