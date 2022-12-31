function Export-WorkflowValue ([string]$name, [string]$value) {
    Write-Output "$name=$value" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
}

function New-NormalizedDirectory ([string]$name, [string]$path) {
    $value = (New-Item -Path $path -ItemType Directory -Force).FullName
    Export-WorkflowValue $name $value
    Return $value
}

function Export-OutputValue ([string]$name, [string]$value) {
    Write-Output "$name=$value" | Out-File -FilePath $env:GITHUB_OUTPUT -Encoding utf8 -Append
}

$is_release = $Env:GITHUB_REF -match '^refs/tags/v\d+\.\d+\.\d+\w*-\d+\.\d+\.\d+$'
Export-OutputValue 'is_release' $is_release

$platform = $Args[0]
$root = (Resolve-Path -LiteralPath $Env:GITHUB_WORKSPACE).Path
$source = (Resolve-Path -LiteralPath $root).Path
Set-Location $source

$git_origin_url = (git config --get remote.origin.url)
$git_master_ref = (git ls-remote "$git_origin_url" master) -split '\s+' | select -First 1
git merge-base --is-ancestor "$git_master_ref" HEAD
If ($LASTEXITCODE -ne 0) {
    git fetch --deepen 100 origin master:master
    $git_master_ref = (git merge-base master $Env:GITHUB_SHA)
}
$git_describe = git describe --tags --dirty --always --exclude "latest-*"
$git_describe_master = git describe --always --tags $git_master_ref

Export-WorkflowValue 'PROJECT_GIT_ORIGIN_URL' $git_origin_url
Export-WorkflowValue 'PROJECT_GIT_MASTER_REF' $git_master_ref
Export-WorkflowValue 'PROJECT_GIT_DESCRIBE' $git_describe
Export-WorkflowValue 'PROJECT_GIT_DESCRIBE_MASTER' $git_describe_master
Export-WorkflowValue 'PROJECT_SOURCE' $source

Export-OutputValue 'git_describe' $git_describe

$build = New-NormalizedDirectory 'PROJECT_BUILD' (Join-Path $root -ChildPath "build\Release-$platform" )
$build_relative_unix = "build/Release-$platform"

Export-WorkflowValue 'PROJECT_TOOLS' (Join-Path $root -ChildPath 'tools')
Export-WorkflowValue 'PROJECT_TOOLS_TMP' (Join-Path $root -ChildPath 'tools.tmp')

$output_name = "dunelegacy-$platform-$git_describe"
$zip_name = $output_name + '.zip'
$msi_zip_name = 'MSI-' + $output_name + '.zip'
$exe_zip_name = 'Setup-' + $output_name + '.zip'

Export-WorkflowValue 'PROJECT_OUTPUT_NAME' $output_name
Export-WorkflowValue 'PROJECT_ZIP' (Join-Path $build -ChildPath $zip_name)
Export-WorkflowValue 'PROJECT_ZIP_RELATIVE_UNIX' ($build_relative_unix + "/" + $zip_name)
Export-WorkflowValue 'PROJECT_ZIP_NAME' $zip_name
Export-WorkflowValue 'PROJECT_MSI' (Join-Path $build -ChildPath ($output_name + '.msi'))
Export-WorkflowValue 'PROJECT_MSI_ZIP' (Join-Path $build -ChildPath $msi_zip_name)
Export-WorkflowValue 'PROJECT_MSI_ZIP_NAME' $msi_zip_name
Export-WorkflowValue 'PROJECT_INSTALLER' (Join-Path $build -ChildPath ($output_name + '.exe'))
Export-WorkflowValue 'PROJECT_INSTALLER_ZIP' (Join-Path $build -ChildPath $exe_zip_name)
Export-WorkflowValue 'PROJECT_INSTALLER_ZIP_NAME' $exe_zip_name
Export-WorkflowValue 'PROJECT_LATEST_TAG' "latest-$platform"
Export-WorkflowValue 'PROJECT_VCPKG_TRIPLET' "$platform-windows-ltcg"
