from setuptools import setup

package_name = 'so101_driver'

setup(
    name=package_name,
    version='1.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='tjdwlswkd21',
    maintainer_email='tjdwlswkd21@gmail.com',
    description='SO-101 LeRobot 드라이버 노드',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'driver_node = so101_driver.driver_node:main',
            'capture_home = so101_driver.capture_home:main',
        ],
    },
)
