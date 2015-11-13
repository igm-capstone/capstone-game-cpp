prefix="Rig3D$1"
echo $prefix

 if [ -z "$1" ]
then
    git subtree pull --prefix Rig3D https://github.com/igm-capstone/Rig3D.git master --squash
fi

cat $prefix/.gitmodules | grep -B1 url | while read line
do
    linearr=($line)

    case ${linearr[0]} in
        path)
            repopath=${linearr[2]}
            ;;
        url)
            #git rm -r $prefix/$repopath
            #git commit -m"Removed submodule $prefix/$repopath folder to add subtree"
            git subtree pull --prefix $prefix/$repopath ${linearr[2]} master --squash

            if [ -f "$prefix/$repopath/.gitmodules" ]
            then
                rig3d-update-subtrees.sh "/$repopath"
            fi
            ;;
    esac

done